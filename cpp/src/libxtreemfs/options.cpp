/*
 * Copyright (c) 2010-2011 by Patrick Schaefer, Zuse Institute Berlin
 *                    2011 by Michael Berlin, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

#include "libxtreemfs/options.h"

#include <boost/program_options/cmdline.hpp>
#include <iostream>
#include <string>

#include "rpc/ssl_options.h"
#include "libxtreemfs/pbrpc_url.h"
#include "libxtreemfs/xtreemfs_exception.h"
#include "util/logging.h"
#include "xtreemfs/GlobalTypes.pb.h"

using namespace std;
using namespace xtreemfs::pbrpc;
using namespace xtreemfs::util;

namespace po = boost::program_options;
namespace style = boost::program_options::command_line_style;

namespace xtreemfs {

Options::Options()
    : general_("General options"),
      optimizations_("Optimizations"),
      error_handling_("Error Handling options"),
      ssl_options_("SSL options"),
      grid_options_("Grid Support options"),
      xtreemfs_advanced_options_("XtreemFS Advanced options") {
  // Version information.
  // If you change this, do not forget to change this also in xtfsutil.cpp!
  version_string = "1.3.0 (RC1, Tasty Tartlet)";

  // XtreemFS URL Options.
  xtreemfs_url = "";
  service_address = "";
  volume_name = "";
  protocol = "";
  mount_point = "";

  // General options.
  log_level_string = "WARN";
  log_file_path = "";
  show_help = false;
  empty_arguments_list = false;
  show_version = false;

  // Optimizations.
  metadata_cache_size = 100000;
  metadata_cache_ttl_s = 120;
  //TODO(mberlin): Reenable async writes when retry support is completed.
  max_writeahead = 0; // 10 * 128 * 1024;  // 10 default objects = 1280kB = 1.25MB.
  max_writeahead_requests = 10;  // Only 10 pending requests allowed by default.
  readdir_chunk_size = 1024;

  // Error Handling options.
  // Depending on the values max{_read|_write}_tries and and (retry_delay_s or
  // (connect_|request_)timeout_s), the _maximum_ time an operation does block
  // is in the range:
  // [(max_tries-1) * retry_delay_s, max_tries * connect_timeout_s]
  // (assuming retry_delay_s <= connect_timeout_s).
  //
  // Example: If there is only one replica available and the connect
  //          attempt does always fail immediately (for instance because the
  //          host is up and it refuses the connection), then the client does
  //          wait retry_delay_s seconds between two attempts, in total
  //          (max_tries-1) * retry_delay_s (-1 because it does not wait after
  //          the last failed attempt).
  //
  //          However, if the attempt does not fail immediately (for instance
  //          the host is not up and we have to wait for the timeout for the
  //          request), the client will wait at least connect_timeout_s or
  //          request_timeout_s seconds for the completion of every request.
  //          The timeout values and retry_delay_s do not add up, i.e. the
  //          client will only wait for the maximum of both.
  //          In total, the maximum time will not exceed max_tries *
  //          (connect_|request_)timeout_s).
  //
  //          With the default values, an unsuccessful operation may block
  //          between ~10 and 40 minutes: [(15-1) seconds * 40 tries,
  //                                       60 secs * 40].
  max_tries = 40;
  max_read_tries = 40;
  max_write_tries = 40;
  retry_delay_s = 15;
  connect_timeout_s = 60;
  request_timeout_s = 60;
  linger_timeout_s = 600;  // 10 Minutes.
  interrupt_signal = 0;  // Disable interruption support by default.

  // SSL options.
  ssl_pem_cert_path = "";
  ssl_pem_path = "";
  ssl_pem_key_pass = "";
  ssl_pkcs12_path = "";
  ssl_pkcs12_pass = "";

  // Grid Support options.
  grid_ssl = false;
  grid_auth_mode_globus = false;
  grid_auth_mode_unicore = false;
  grid_gridmap_location = "";
  grid_gridmap_location_default_globus = "/etc/grid-security/grid-mapfile";
  grid_gridmap_location_default_unicore = "/etc/grid-security/d-grid_uudb";
  grid_gridmap_reload_interval_m = 60;  // 60 Minutes = 1 Hour.

  // Advanced XtreemFS options.
  periodic_file_size_updates_interval_s = 60;  // Default: 1 Minute.
  periodic_xcap_renewal_interval_s = 60;  // Default: 1 Minute.

  // User mapping.
  user_mapping_type = UserMapping::kUnix;

  all_descriptions_initialized_ = false;
}

void Options::GenerateProgramOptionsDescriptions() {
  if (all_descriptions_initialized_) {
    return;
  }

  // Init boost::program_options specific things, define options.
  general_.add_options()
    ("log-level,d",
        po::value(&log_level_string)->default_value(log_level_string),
        "EMERG|ALERT|CRIT|ERR|WARNING|NOTICE|INFO|DEBUG")
    ("log-file-path,l",
        po::value(&log_file_path)->default_value(log_file_path),
        "Path to log file.")
    ("help,h",
        po::value(&show_help)->zero_tokens(),
        "Display this text.")
    ("version,V",
        po::value(&show_version)->zero_tokens(),
        "Shows the version number.");

  optimizations_.add_options()
    ("metadata-cache-size",
        po::value(&metadata_cache_size)->default_value(metadata_cache_size),
        "Number of entries which will be cached."
        "\n(Set to 0 to disable the cache.)")
    ("metadata-cache-ttl-s",
        po::value(&metadata_cache_ttl_s)->default_value(metadata_cache_ttl_s),
        "Time to live after which cached entries will expire.")
    ("max-writeahead",
        po::value(&max_writeahead)->default_value(max_writeahead),
        "Maximum number of pending written bytes per file. Set this to 0"
        " to completely disable asynchronous writes)")
    ("max-writeahead-requests",
        po::value(&max_writeahead_requests)
            ->default_value(max_writeahead_requests),
        "Maximum number of pending write requests per file (Asynchronous writes"
        " will block if this or max-writeahead is reached first).")
        //TODO(bjko): Insert new OSD parameter.
//        "\nIf you increase this value, please take care of the value "
//        " in the OSD config and do not set it to high to avoid flooding.")
    ("readdir-chunk-size",
        po::value(&readdir_chunk_size)->default_value(readdir_chunk_size),
        "Number of entries requested per readdir.");

  error_handling_.add_options()
    ("max-tries",
        po::value(&max_tries)->default_value(max_tries),
        "Maximum number of attempts to send a request (0 means infinite).")
    ("max-read-tries",
        po::value(&max_read_tries)->default_value(max_read_tries),
        "Maximum number of attempts to execute a read command (0 means infinite"
        ")."
#ifdef __linux
        "\n(If you use Fuse it's not possible to interrupt a read request, i.e."
        " do not set this value too high or to infinite.)"
#endif  // __linux
        )
    ("max-write-tries",
        po::value(&max_write_tries)->default_value(max_write_tries),
        "Maximum number of attempts to execute a write command (0 means "
        "infinite)."
#ifdef __linux
        "\n(Unlike read requests, write requests can get interrupted in "
        "Fuse.)"
#endif  // __linux
        )
    ("retry-delay",
        po::value(&retry_delay_s)->default_value(retry_delay_s),
        "Wait time after a request failed until next attempt (in seconds).")
    ("interrupt-signal",
        po::value(&interrupt_signal)->default_value(interrupt_signal),
        "Retry of a request is interrupted if this signal is sent "
        "(set to 0 to disable it)."
#ifdef __APPLE__
        " (This option has no effect with MacFuse as it supports to interrupt"
        " all requests by default.)"
#endif  // __APPLE__
#ifdef __linux
        "\n(If not disabled and Fuse is used, -o intr and -o intr_signal=10 "
        "(=SIGUSR1) will be passed to Fuse by default.)"
#endif  // __linux
        )
    ("connect-timeout",
        po::value(&connect_timeout_s)->default_value(connect_timeout_s),
        "Timeout after which a connection attempt will be retried "
        "(in seconds).")
    ("request-timeout",
        po::value(&request_timeout_s)->default_value(request_timeout_s),
        "Timeout after which a request will be retried (in seconds).")
    ("linger-timeout",
        po::value(&linger_timeout_s)->default_value(linger_timeout_s),
        "Time after which idle connections will be closed (in seconds).");

  ssl_options_.add_options()
    ("pem-certificate-file-path",
        po::value(&ssl_pem_cert_path)->default_value(ssl_pem_cert_path),
        "PEM certificate file path")
    ("pem-private-key-file-path",
        po::value(&ssl_pem_path)->default_value(ssl_pem_path),
        "PEM private key file path")
    ("pem-private-key-passphrase",
        po::value(&ssl_pem_key_pass)->default_value(ssl_pem_key_pass),
        "PEM private key passphrase")
    ("pkcs12-file-path",
        po::value(&ssl_pkcs12_path)->default_value(ssl_pkcs12_path),
        "PKCS#12 file path")
    ("pkcs12-passphrase",
        po::value(&ssl_pkcs12_pass)->default_value(ssl_pkcs12_pass),
        "PKCS#12 passphrase");

  grid_options_.add_options()
    ("grid-ssl",
        po::value(&grid_ssl)->zero_tokens(),
        "Explicitily use the XtreemFS Grid-SSL mode. Same as specifying "
        "pbrpcg:// in the volume URL.")
    ("globus-gridmap",
        po::value(&grid_auth_mode_globus)->zero_tokens(),
        "Authorize using globus gridmap file.")
    ("unicore-gridmap",
        po::value(&grid_auth_mode_unicore)->zero_tokens(),
        "Authorize using unicore gridmap file.")
    ("gridmap-location",
        po::value(&grid_gridmap_location)->default_value(grid_gridmap_location),
        string("Location of the gridmap file.\n"
        "unicore default: " + grid_gridmap_location_default_unicore + "\n"
        "globus default: " + grid_gridmap_location_default_globus).c_str())
    ("gridmap-reload-interval-m",
        po::value(&grid_gridmap_reload_interval_m)
            ->default_value(grid_gridmap_reload_interval_m),
        "Interval (in minutes) after which the gridmap file will be checked for"
        " changes and reloaded if necessary.");

  xtreemfs_advanced_options_.add_options()
    ("periodic-filesize-update-interval",
        po::value(&periodic_file_size_updates_interval_s),
        "Pause time (in seconds) between two invocations of the thread which "
        "writes back file size updates to the MRC in the background.")
    ("periodic-",
        po::value(&periodic_xcap_renewal_interval_s),
        "Pause time (in seconds) between two invocations of the thread which "
        "renews the XCap of all open file handles.");

  all_descriptions_.add(general_).add(optimizations_).add(error_handling_)
      .add(ssl_options_).add(grid_options_);
  // These options are not shown in the "-h" output to not confuse the user.
  hidden_descriptions_.add(xtreemfs_advanced_options_);

  all_descriptions_initialized_ = true;
}

std::vector<std::string> Options::ParseCommandLine(int argc, char** argv) {
  GenerateProgramOptionsDescriptions();

  // Parse command line.
  boost::program_options::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv)
        .options(all_descriptions_)
        .allow_unregistered()
        .style(style::default_style & ~style::allow_guessing)
        .run(), vm);
    po::notify(vm);
  } catch(const std::exception& e) {
    // Rethrow boost errors due to invalid command line parameters.
    throw InvalidCommandLineParametersException(string(e.what()));
  }
  po::parsed_options parsed = po::command_line_parser(argc, argv)
  .options(all_descriptions_)
  .allow_unregistered()
  .style(style::default_style & ~style::allow_guessing)
  .run();

  if (metadata_cache_size < readdir_chunk_size && metadata_cache_size != 0) {
    cerr << "Warning: Please set the metadata cache size at least as high as "
            "the readdir chunk size. (Currently: " << metadata_cache_size <<
            " < " << readdir_chunk_size << "). Otherwise you might experience"
            " a degraded performance."
         << endl << endl;
  }

  if (max_writeahead_requests < 1) {
    throw InvalidCommandLineParametersException("The maximum number of pending"
        " asynchronous writes (max-writeahead-requests) must be greater or"
        " equal 1. If you want to completely disable asynchronous writes,"
        " please set max-writeahead to 0. The value of max-writeahead-requests"
        " will be ignored then.");
  }

  if (max_writeahead != 0 && max_writeahead < 128 * 1024) {
    throw InvalidCommandLineParametersException("Please specify a writeahead"
        " size which is at least as high as the default object size of"
        " 131072 byes (128 kB).");
  }

  if (max_writeahead != 0 &&
      (max_writeahead_requests * 128 * 1024 > max_writeahead)) {
    cerr << "Information: The value max-writeahead-requests may be ineffective"
            " if you are writing buffers of a multiple of the object size"
            " (default 128kB), as it does currently limit the number of"
            " requests to:\n\t"
         << max_writeahead << " / " << " 128kB default object size = "
         << (max_writeahead / 128 / 1024) << "\n\t(which is lower than the set"
            " maximum number of requests: " << max_writeahead_requests << ").\n"
         << "The recommended max-writeahead for the current maximum number of"
            " pending writes is: " << (max_writeahead_requests * 128 * 1024)
         << endl << endl;
  }
  if (max_writeahead != 0 &&
      (max_writeahead > max_writeahead_requests * 128 * 1024)) {
    cerr << "Information: The value max-writeahead may be ineffective as the"
            " allowed maximum number of pending writes (max-writeahead-requests"
            " = " << max_writeahead_requests << ") does limit the maximum"
            " writeahead considering the default object size 128kB:\n\t"
         << "max write ahead of " << max_writeahead << " > " << "128kB default"
            " object size * " << max_writeahead_requests << " max write"
            " requests (= " << (128 * 1024 * max_writeahead_requests) <<
            " effective writeahead)."
         << endl << endl;
  }

  // Show help if no arguments given.
  if (argc == 1) {
    empty_arguments_list = true;
  }

  if (grid_auth_mode_globus && grid_auth_mode_unicore) {
    throw InvalidCommandLineParametersException("You can only use a Globus "
        "OR a Unicore gridmap file at the same time.");
  }
  if (grid_auth_mode_globus) {
    user_mapping_type = UserMapping::kGlobus;
    if (grid_gridmap_location.empty()) {
      grid_gridmap_location = grid_gridmap_location_default_globus;
    }
  }
  if (grid_auth_mode_unicore) {
    user_mapping_type = UserMapping::kUnicore;
    if (grid_gridmap_location.empty()) {
      grid_gridmap_location = grid_gridmap_location_default_unicore;
    }
  }

  // Return unparsed options.
  return po::collect_unrecognized(parsed.options, po::include_positional);
}

void Options::ParseURL(XtreemFSServiceType service_type) {
  int default_port;
  switch(service_type) {
    case kMRC:
      default_port = MRC_PBRPC_PORT_DEFAULT;
      break;
    case kDIR:
    default:
      default_port = DIR_PBRPC_PORT_DEFAULT;
      break;
  }

  PBRPCURL url_parser;
  url_parser.parseURL(xtreemfs_url, PBRPCURL::SCHEME_PBRPC, default_port);
  volume_name = url_parser.volume();
  service_address = url_parser.getAddress();
  protocol = url_parser.scheme();
}

std::string Options::ShowCommandLineHelp() {
  GenerateProgramOptionsDescriptions();
  ostringstream stream;
  stream << all_descriptions_;
  return stream.str();
}

std::string Options::ShowCommandLineHelpVolumeCreation() {
  GenerateProgramOptionsDescriptions();
  ostringstream stream;
  stream << general_ << endl
         << ssl_options_ << endl
         << grid_options_;
  return stream.str();
}

std::string Options::ShowCommandLineHelpVolumeDeletionAndListing() {
  GenerateProgramOptionsDescriptions();
  ostringstream stream;
  stream << general_ << endl
         << ssl_options_;
  return stream.str();
}

std::string Options::ShowVersion(const std::string& component) {
  return component + " " + version_string;
}

xtreemfs::rpc::SSLOptions* Options::GenerateSSLOptions() const {
  xtreemfs::rpc::SSLOptions* opts = NULL;
  if (!ssl_pem_cert_path.empty() || !ssl_pkcs12_path.empty()) {
    opts = new xtreemfs::rpc::SSLOptions(
        ssl_pem_path, ssl_pem_cert_path, ssl_pem_key_pass,  // PEM.
        ssl_pkcs12_path, ssl_pkcs12_pass,  // PKCS12.
        boost::asio::ssl::context::pem,
        grid_ssl || protocol == PBRPCURL::SCHEME_PBRPCG);
  }

  return opts;
}

}  // namespace xtreemfs
