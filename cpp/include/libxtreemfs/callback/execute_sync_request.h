/*
 * Copyright (c) 2011 by Michael Berlin, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

#ifndef CPP_INCLUDE_LIBXTREEMFS_CALLBACK_EXECUTE_SYNC_REQUEST_H_
#define CPP_INCLUDE_LIBXTREEMFS_CALLBACK_EXECUTE_SYNC_REQUEST_H_

#ifdef __linux
#include <csignal>
#endif

#include <ctime>

#include <algorithm>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <google/protobuf/descriptor.h>
#include <iostream>
#include <string>

#include "libxtreemfs/options.h"
#include "libxtreemfs/uuid_iterator.h"
#include "libxtreemfs/uuid_resolver.h"
#include "libxtreemfs/xtreemfs_exception.h"
#include "pbrpc/RPC.pb.h"
#include "rpc/sync_callback.h"
#include "util/error_log.h"
#include "util/logging.h"

namespace xtreemfs {

namespace rpc {
class ClientRequestCallbackInterface;
}  // namespace rpc

/** Is not NULL, if current thread shall get interrupted. */
extern boost::thread_specific_ptr<int> intr_pointer;

/** Sets intr_pointer to interrupt execution of the sync request. */
void InterruptSyncRequest(int signal);

/** Retries to execute the synchronous request "sync_function" up to "options.
 *  max_tries" times and may get interrupted. The "uuid_iterator" object is used
 *  to retrieve UUIDs or mark them as failed.
 *  If uuid_iterator_has_addresses=true, the resolving of the UUID is skipped
 *  and the string retrieved by uuid_iterator->GetUUID() is used as address.
 *  (in this case uuid_resolver may be NULL).
 *
 *  The interrupt handler is only registered, if a signal "options.
 *  interrupt_signal" is set.
 *
 *  The parameter delay_last_attempt should be set true, if this method is
 *  called with max_tries = 1 and one does the looping over the retries on its
 *  own (for instance in FileHandleImplementation::AcquireLock). If set to false
 *  this method would return immediately after the _last_ try and the caller would
 *  have to ensure the delay of options.retry_delay_s on its own.
 */
template<class ReturnMessageType, class F>
    ReturnMessageType ExecuteSyncRequest(F sync_function,
                                         UUIDIterator* uuid_iterator,
                                         UUIDResolver* uuid_resolver,
                                         int max_tries,
                                         const Options& options,
                                         bool uuid_iterator_has_addresses,
                                         bool delay_last_attempt) {
  assert(uuid_iterator_has_addresses || uuid_resolver);

#ifdef __linux
  // Ignore the signal if no previous signal was found.
  sighandler_t previous_signal_handler = SIG_IGN;
  if (options.interrupt_signal) {
    // Clear current interruption state.
    intr_pointer.reset(NULL);
    // Register signal handler to allow an interruption.
    previous_signal_handler = signal(options.interrupt_signal,
                                     InterruptSyncRequest);
  }
#endif

  int attempt = 0;
  bool interrupted = false;
  ReturnMessageType response = NULL;
  std::string service_uuid = "";
  std::string service_address;
  // Retry unless maximum tries reached or interrupted.
  while ((++attempt <= max_tries || max_tries == 0) && !interrupted) {
    // Delete any previous response;
    if (response != NULL) {
      response->DeleteBuffers();
      delete response;
    }

    // Resolve UUID first.
    if (uuid_iterator_has_addresses) {
      uuid_iterator->GetUUID(&service_address);
    } else {
      uuid_iterator->GetUUID(&service_uuid);
      uuid_resolver->UUIDToAddress(service_uuid, &service_address);
    }

    // Execute request.
    timeval request_sent, current_time;
    gettimeofday(&request_sent, 0);
    // TODO(mberlin): Check if it is necessary to copy the latest XCap from
    //                a known file before executing the request.
    response = sync_function(service_address);

    // Check response.
    if (response->HasFailed()) {
      // An error has happened. Differ between communication problems (retry
      // allowed) and application errors (need to pass to the caller).

      // Special handling of an UUID redirection.
      if (response->error()->error_type() == xtreemfs::pbrpc::REDIRECT) {
        assert(response->error()->has_redirect_to_server_uuid());
        uuid_iterator->SetCurrentUUID(
            response->error()->redirect_to_server_uuid());
        // Log the redirect.
        xtreemfs::util::LogLevel level = xtreemfs::util::LEVEL_INFO;
        std::string error;
        if (uuid_iterator_has_addresses) {
          error = "The server: " + service_address
                + " redirected to the current master: "
                + response->error()->redirect_to_server_uuid()
                + " at attempt: " + boost::lexical_cast<std::string>(attempt);
        } else {
          error = "The server with the UUID: " + service_uuid
                + " redirected to the current master with the UUID: "
                + response->error()->redirect_to_server_uuid()
                + " at attempt: " + boost::lexical_cast<std::string>(attempt);
        }
        if (xtreemfs::util::Logging::log->loggingActive(level)) {
          xtreemfs::util::Logging::log->getLog(level) << error << std::endl;
        }
        xtreemfs::util::ErrorLog::error_log->AppendError(error);

        if (max_tries != 0 && attempt == max_tries) {
          // This was the last retry, but we give it another chance.
          max_tries++;
        }

        // Do a fast retry and do not delay until the next attempt.
        continue;
      }

      // Retry (and delay) only if at least one retry is left
      if (((attempt < max_tries || max_tries == 0) ||
           //                      or this last retry should be delayed
           (attempt == max_tries && delay_last_attempt)) &&
          // AND it is an recoverable error.
          (response->error()->error_type() == xtreemfs::pbrpc::IO_ERROR ||
           response->error()->error_type()
               == xtreemfs::pbrpc::INTERNAL_SERVER_ERROR)) {
        // Mark the current UUID as failed and get the next one.
        if (uuid_iterator_has_addresses) {
          uuid_iterator->MarkUUIDAsFailed(service_address);
          uuid_iterator->GetUUID(&service_address);
        } else {
          uuid_iterator->MarkUUIDAsFailed(service_uuid);
          uuid_iterator->GetUUID(&service_uuid);
        }

        // Log only the first retry.
        if (attempt == 1 && max_tries != 1) {
          std::string retries_left = max_tries == 0 ? "infinite"
              : boost::lexical_cast<std::string>(max_tries - attempt);
          xtreemfs::util::Logging::log->getLog(xtreemfs::util::LEVEL_ERROR)
              << "Got no response from server " << service_uuid << " ("
              << service_address << "), retrying ("
              << retries_left << " attempts left, waiting at least "
              << options.retry_delay_s << " seconds between two attempts)."
              << std::endl;
        }

        // If the request did return before the timeout was reached, wait until
        // the timeout is up to avoid flooding.
        boost::int64_t delay_time_left;
        do {
          if (options.interrupt_signal && intr_pointer.get() != NULL) {
            // Stop retrying if interrupted between two attempts.
            interrupted = true;
            break;
          }
          gettimeofday(&current_time, 0);
          delay_time_left =
              static_cast<boost::int64_t>(options.retry_delay_s) * 1000000
              - ((current_time.tv_sec * 1000000 + current_time.tv_usec) -
                 (request_sent.tv_sec * 1000000 + request_sent.tv_usec));
          if (delay_time_left > 0) {
            boost::this_thread::sleep(
                boost::posix_time::millisec(100));
          }
        } while (delay_time_left > 0);
      } else {
        break;  // Do not retry if error occured - throw exception.
      }
    } else {
      // No error happened, check for possible interruption.
    }

    // Have we been interrupted?
    if (options.interrupt_signal && intr_pointer.get() != NULL) {
      if (xtreemfs::util::Logging::log->loggingActive(
              xtreemfs::util::LEVEL_DEBUG)) {
        xtreemfs::util::Logging::log->getLog(xtreemfs::util::LEVEL_DEBUG)
            << "Caught interrupt, aborting sync request." << std::endl;
      }
      intr_pointer.reset(NULL);
      // Clear the current response.
      if (response != NULL) {
        response->DeleteBuffers();
      }
      // Free response.
      delete response;
      response = NULL;
      break;  // Do not retry if interrupted.
    }
    if (response != NULL && !response->HasFailed()) {
      break;  // Do not retry if request was successful.
    }
  }
#ifdef __linux
  // Remove signal handler.
  if (options.interrupt_signal) {
    signal(options.interrupt_signal, previous_signal_handler);
  }
#endif

  // Request was successful.
  if (response && !response->HasFailed()) {
    return response;
  }

  // Output number of retries if not failed at the first retry.
  std::string retry_count_msg;
  if (attempt > 1) {
    retry_count_msg = ". Request finally failed after: "
       + boost::lexical_cast<std::string>(attempt) + " attempts.";
  } else {
    retry_count_msg = "";
  }
  // Max attempts reached or non-IO error seen. Throw an exception.
  if (response != NULL) {
    // Copy error information in order to delete buffers before the throw.
    xtreemfs::pbrpc::RPCHeader::ErrorResponse* error_resp = response->error();
    const xtreemfs::pbrpc::ErrorType error_type = error_resp->error_type();
    const std::string error_message = error_resp->error_message();
    const xtreemfs::pbrpc::POSIXErrno posix_errno = error_resp->posix_errno();
    std::string redirect_to_server_uuid = "";
    if (error_resp->has_redirect_to_server_uuid()) {
        redirect_to_server_uuid = error_resp->redirect_to_server_uuid();
    }

    // Free buffers.
    response->DeleteBuffers();
    delete response;

    // By default all errors are logged as errors.
    xtreemfs::util::LogLevel level = xtreemfs::util::LEVEL_ERROR;
    // String for complete error text which will be logged.
    std::string error;

    // Throw an exception.
    switch (error_type) {
      case xtreemfs::pbrpc::ERRNO:  {
        // Posix errors are usually not logged as errors.
        level = xtreemfs::util::LEVEL_INFO;
        if (posix_errno == xtreemfs::pbrpc::POSIX_ERROR_ENOENT) {
          level = xtreemfs::util::LEVEL_DEBUG;
        }
        std::string posix_errono_string
            = boost::lexical_cast<std::string>(posix_errno);
        const ::google::protobuf::EnumValueDescriptor* enum_desc =
            xtreemfs::pbrpc::POSIXErrno_descriptor()->
                FindValueByNumber(posix_errno);
        if (enum_desc) {
            posix_errono_string = enum_desc->name();
        }
        error = "The server " + service_uuid + " (" + service_address
            + ") denied the requested operation."
              " Error Value: " + posix_errono_string
            + " Error message: " + error_message
            + retry_count_msg;
        if (xtreemfs::util::Logging::log->loggingActive(level)) {
          xtreemfs::util::Logging::log->getLog(level) << error << std::endl;
        }
        xtreemfs::util::ErrorLog::error_log->AppendError(error);
        throw PosixErrorException(posix_errno, error);
      }
      case xtreemfs::pbrpc::IO_ERROR:  {
        error = "The client encountered a communication error sending a request"
            " to the server: " + service_uuid + " (" + service_address + ")."
            " Error: " + error_message + retry_count_msg;
        if (xtreemfs::util::Logging::log->loggingActive(level)) {
          xtreemfs::util::Logging::log->getLog(level) << error << std::endl;
        }
        xtreemfs::util::ErrorLog::error_log->AppendError(error);
        throw IOException(error_message);
      }
      case xtreemfs::pbrpc::INTERNAL_SERVER_ERROR:  {
        error = "The server " + service_uuid + " (" + service_address + ") "
            "returned an internal server error: " + error_message
            + retry_count_msg;
        if (xtreemfs::util::Logging::log->loggingActive(level)) {
          xtreemfs::util::Logging::log->getLog(level) << error << std::endl;
        }
        xtreemfs::util::ErrorLog::error_log->AppendError(error);
        throw InternalServerErrorException(error_message);
      }
      case xtreemfs::pbrpc::REDIRECT:  {
        throw XtreemFSException("This error (A REDIRECT error was not handled "
            "and retried but thrown instead) should never happen. Report this");
      }
      default:  {
        std::string error_type_name
            = boost::lexical_cast<std::string>(error_type);
        const ::google::protobuf::EnumValueDescriptor* enum_desc =
            xtreemfs::pbrpc::ErrorType_descriptor()->
                FindValueByNumber(error_type);
        if (enum_desc) {
          error_type_name = enum_desc->name();
        }
        error = "The server " + service_uuid + " (" + service_address + ") "
            "returned an error: " + error_type_name
            + " Error: " + error_message + retry_count_msg;
        if (xtreemfs::util::Logging::log->loggingActive(level)) {
          xtreemfs::util::Logging::log->getLog(level) << error << std::endl;
        }
        xtreemfs::util::ErrorLog::error_log->AppendError(error);
        throw XtreemFSException(error);
      }
    }
  } else {
    // No Response given, probably interrupted.
    throw PosixErrorException(
        xtreemfs::pbrpc::POSIX_ERROR_EINTR,
        "The operation (sending a request to the server " + service_uuid + " ("
        + service_address + ")) was aborted by the user at attempt: "
        // attempt + 1 because the interrupt is only possible after the request
        // came back.
        + boost::lexical_cast<std::string>(attempt + 1) + ".");
  }
}

/** Executes the request without delaying the last try. */
template<class ReturnMessageType, class F>
    ReturnMessageType ExecuteSyncRequest(F sync_function,
                                         UUIDIterator* uuid_iterator,
                                         UUIDResolver* uuid_resolver,
                                         int max_tries,
                                         const Options& options) {
  return ExecuteSyncRequest<ReturnMessageType>(sync_function,
                                               uuid_iterator,
                                               uuid_resolver,
                                               max_tries,
                                               options,
                                               false,
                                               false);
}

}  // namespace xtreemfs

#endif  // CPP_INCLUDE_LIBXTREEMFS_CALLBACK_EXECUTE_SYNC_REQUEST_H_
