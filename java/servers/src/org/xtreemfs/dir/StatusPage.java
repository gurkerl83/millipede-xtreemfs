/*
 * Copyright (c) 2009-2011 by Bjoern Kolbeck,
 *               Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

package org.xtreemfs.dir;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.Iterator;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Map.Entry;

import org.xtreemfs.babudb.BabuDBFactory;
import org.xtreemfs.babudb.api.database.Database;
import org.xtreemfs.babudb.api.exception.BabuDBException;
import org.xtreemfs.common.HeartbeatThread;
import org.xtreemfs.common.config.ServiceConfig;
import org.xtreemfs.dir.data.AddressMappingRecord;
import org.xtreemfs.dir.data.AddressMappingRecords;
import org.xtreemfs.dir.data.ConfigurationRecord;
import org.xtreemfs.dir.data.ServiceRecord;
import org.xtreemfs.foundation.VersionManagement;
import org.xtreemfs.foundation.buffer.BufferPool;
import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.util.OutputUtils;
import org.xtreemfs.osd.vivaldi.VivaldiNode;
import org.xtreemfs.pbrpc.generatedinterfaces.DIRServiceConstants;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.ServiceStatus;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.KeyValuePair;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.VivaldiCoordinates;

/**
 * 
 * @author bjko
 */
public class StatusPage {
    
    private final static String statusPageTemplate;
    
    private enum Vars {
            
            MAXMEM("<!-- $MAXMEM -->"),
            FREEMEM("<!-- $FREEMEM -->"),
            AVAILPROCS("<!-- $AVAILPROCS -->"),
            BPSTATS("<!-- $BPSTATS -->"),
            PORT("<!-- $PORT -->"),
            DEBUG("<!-- $DEBUG -->"),
            NUMCON("<!-- $NUMCON -->"),
            PINKYQ("<!-- $PINKYQ -->"),
            NUMREQS("<!-- $NUMREQS -->"),
            TIME("<!-- $TIME -->"),
            TABLEDUMP("<!-- $TABLEDUMP -->"),
            PROTOVERSION("<!-- $PROTOVERSION -->"),
            VERSION("<!-- $VERSION -->"),
            DBVERSION("<!-- $DBVERSION -->");
        
        private String template;
        
        Vars(String template) {
            this.template = template;
        }
        
        public String toString() {
            return template;
        }
    };
    
    static {
        StringBuffer sb = null;
        try {
            InputStream is = StatusPage.class.getClassLoader().getResourceAsStream(
                "org/xtreemfs/dir/templates/status.html");
            if (is == null) {
                is = StatusPage.class.getClass().getResourceAsStream("../templates/status.html");
            }
            BufferedReader br = new BufferedReader(new InputStreamReader(is));
            sb = new StringBuffer();
            String line = br.readLine();
            while (line != null) {
                sb.append(line + "\n");
                line = br.readLine();
            }
            br.close();
        } catch (Exception ex) {
            Logging.logMessage(Logging.LEVEL_WARN, Category.misc, (Object) null,
                "could not load status page template: %s", OutputUtils.stackTraceToString(ex));
        }
        if (sb == null) {
            statusPageTemplate = "<H1>Template was not found, unable to show status page!</h1>";
        } else {
            statusPageTemplate = sb.toString();
        }
    }
    
    public static String getStatusPage(DIRRequestDispatcher master, DIRConfig config) throws BabuDBException,
        IOException, InterruptedException {
        
        final Database database = master.getDirDatabase();
        
        assert (statusPageTemplate != null);
        
        long time = System.currentTimeMillis();
        
        Iterator<Entry<byte[], byte[]>> iter = database.prefixLookup(DIRRequestDispatcher.INDEX_ID_ADDRMAPS,
            new byte[0], null).get();
        
        StringBuilder dump = new StringBuilder();
        dump
                .append("<br><table width=\"100%\" frame=\"box\"><td colspan=\"2\" class=\"heading\">Address Mapping</td>");
        dump.append("<tr><td class=\"dumpTitle\">UUID</td><td class=\"dumpTitle\">mapping</td></tr>");
        while (iter.hasNext()) {
            Entry<byte[], byte[]> e = iter.next();
            AddressMappingRecords ams = new AddressMappingRecords(ReusableBuffer.wrap(e.getValue()));
            
            final String uuid = new String(e.getKey());
            
            dump.append("<tr><td class=\"uuid\">");
            dump.append(uuid);
            dump.append("</td><td class=\"dump\"><table width=\"100%\"><tr>");
            dump.append("<tr><td><table width=\"100%\">");
            long version = 0;
            for (AddressMappingRecord am : ams.getRecords()) {
                dump.append("<tr><td class=\"mapping\">");
                String endpoint = am.getUri() + " (" + am.getProtocol() + "," + am.getAddress() + ","
                    + am.getPort() + ")";
                dump.append(endpoint);
                dump.append("</td><td class=\"mapping\">");
                dump.append(am.getMatch_network());
                dump.append("</td><td class=\"mapping\">");
                dump.append(am.getTtl_s());
                dump.append("</td></tr>");
                version = am.getVersion();
            }
            dump.append("</table></td></tr>");
            dump.append("<td class=\"version\">version: <b>");
            dump.append(version);
            dump.append("</b></td></tr></table>");
        }
        dump.append("</td></tr></table>");
        
        iter = database.prefixLookup(DIRRequestDispatcher.INDEX_ID_SERVREG, new byte[0], null).get();
        
        dump
                .append("<br><table width=\"100%\" frame=\"box\"><td colspan=\"2\" class=\"heading\">Service Registry</td>");
        dump.append("<tr><td class=\"dumpTitle\">UUID</td><td class=\"dumpTitle\">mapping</td></tr>");
        while (iter.hasNext()) {
            Entry<byte[], byte[]> e = iter.next();
            final String uuid = new String(e.getKey());
            final ServiceRecord sreg = new ServiceRecord(ReusableBuffer.wrap(e.getValue()));
            
            dump.append("<tr><td class=\"uuid\">");
            dump.append(uuid);
            dump.append("</td><td class=\"dump\"><table width=\"100%\">");
            
            dump.append("<tr><td width=\"30%\">");
            dump.append("type");
            dump.append("</td><td><b>");
            dump.append(sreg.getType());
            dump.append("</b></td></tr>");
            
            dump.append("<tr><td width=\"30%\">");
            dump.append("name");
            dump.append("</td><td><b>");
            dump.append(sreg.getName());
            dump.append("</b></td></tr>");
            
            // sort the set of entries
            SortedMap<String, String> sMap = new TreeMap<String, String>();
            for(Entry<String, String> entry: sreg.getData().entrySet())
                sMap.put(entry.getKey(), entry.getValue());
                
            for (Entry<String, String> dataEntry : sMap.entrySet()) {
                dump.append("<tr><td width=\"30%\">");
                dump.append(dataEntry.getKey());
                dump.append("</td><td><b>");
                
                if (dataEntry.getKey().equals("status_page_url")) {
                    dump.append("<a href=\"");
                    dump.append(dataEntry.getValue());
                    dump.append("\">");
                }
                
                dump.append(dataEntry.getValue());
                
                if (dataEntry.getKey().equals("status_page_url")) {
                    dump.append("</a>");
                } else if (dataEntry.getKey().equals("last_updated")) {
                    
                } else if (dataEntry.getKey().equals(HeartbeatThread.STATUS_ATTR)) {
                    ServiceStatus status = ServiceStatus.valueOf(Integer.valueOf(dataEntry.getValue()));
                    switch (status) {
                    case SERVICE_STATUS_AVAIL:
                        dump.append(" (online)");
                        break;
                    case SERVICE_STATUS_TO_BE_REMOVED:
                        dump.append(" (locked for removal)");
                        break;
                    case SERVICE_STATUS_REMOVED:
                        dump.append(" (removed, dead)");
                        break;
                    }
                } else if (dataEntry.getKey().equals("free") || dataEntry.getKey().equals("total")
                    || dataEntry.getKey().endsWith("RAM") || dataEntry.getKey().equals("used")) {
                    dump.append(" bytes (");
                    dump.append(OutputUtils.formatBytes(Long.parseLong(dataEntry.getValue())));
                    dump.append(")");
                } else if (dataEntry.getKey().equals("load")) {
                    dump.append("%");
                } else if (dataEntry.getKey().equals("vivaldi_coordinates")) {
                    final VivaldiCoordinates coord = VivaldiNode.stringToCoordinates(dataEntry.getValue());
                    dump.append(" (");
                    dump.append(coord.getXCoordinate());
                    dump.append(",");
                    dump.append(coord.getYCoordinate());
                    dump.append(" err ");
                    dump.append(coord.getLocalError());
                    dump.append(")");
                }
                dump.append("</b></td></tr>");
            }
            
            dump.append("<tr><td width=\"30%\">");
            dump.append("last updated");
            dump.append("</td><td><b>");
            dump.append(sreg.getLast_updated_s());
            if (sreg.getLast_updated_s() == 0) {
                dump.append(" (service is offline)");
            } else {
                dump.append(" (");
                dump.append(new Date(sreg.getLast_updated_s() * 1000));
                dump.append(")");
                dump.append("</b></td></tr>");
            }
            
            dump.append("<td></td><td class=\"version\">version: <b>");
            dump.append(sreg.getVersion());
            dump.append("</b></td></table></td></tr>");
        }
        
        dump.append("</td></tr></table>");
        // Configuration part
        
        iter = database.prefixLookup(DIRRequestDispatcher.INDEX_ID_CONFIGURATIONS, new byte[0], null).get();
        
        dump
                .append("<br><table width=\"100%\" frame=\"box\"><td colspan=\"2\" class=\"heading\">Configurations</td>");
        dump
                .append("<tr><td class=\"dumpTitle\">UUID</td><td class=\"dumpTitle\">Configuration Parameter</td></tr>");
        
        while (iter.hasNext()) {
            Entry<byte[], byte[]> e = iter.next();
            final String uuid = new String(e.getKey());
            final ConfigurationRecord conf = new ConfigurationRecord(ReusableBuffer.wrap(e.getValue()));
            
            dump.append("<tr><td class=\"uuid\">");
            dump.append(uuid);
            dump.append("</td><td class=\"dump\"><table width=\"100%\">");
            
            Collections.sort(conf.getData(), new Comparator<KeyValuePair>() {
                public int compare(KeyValuePair o1, KeyValuePair o2) {
                    return o1.getKey().compareTo(o2.getKey());
                }
            });
            
            for (KeyValuePair kvp : conf.getData()) {
                dump.append("<tr><td width=\"30%\">");
                dump.append(kvp.getKey());
                dump.append("</td><td><b>");
                
                dump.append(kvp.getKey().equals(ServiceConfig.Parameter.ADMIN_PASSWORD.getPropertyString())
                    || kvp.getKey().equals(ServiceConfig.Parameter.CAPABILITY_SECRET.getPropertyString())
                    || kvp.getKey().equals(
                        ServiceConfig.Parameter.SERVICE_CREDS_PASSPHRASE.getPropertyString())
                    || kvp.getKey().equals(
                        ServiceConfig.Parameter.TRUSTED_CERTS_PASSPHRASE.getPropertyString()) ? "*******"
                    : kvp.getValue());
                dump.append("</b></td></tr>");
            }
            
            dump.append("<td></td><td class=\"version\">version: <b>");
            dump.append(conf.getVersion());
            dump.append("</b></td></table></td></tr>");
        }
        dump.append("</b></td></table></td></tr>");
        dump.append("</table>");
        
        String tmp = null;
        try {
            tmp = statusPageTemplate.replace(Vars.AVAILPROCS.toString(), Runtime.getRuntime()
                    .availableProcessors()
                + " bytes");
        } catch (Exception e) {
            tmp = statusPageTemplate;
        }
        tmp = tmp.replace(Vars.FREEMEM.toString(), Runtime.getRuntime().freeMemory() + " bytes");
        tmp = tmp.replace(Vars.MAXMEM.toString(), Runtime.getRuntime().maxMemory() + " bytes");
        tmp = tmp.replace(Vars.BPSTATS.toString(), BufferPool.getStatus());
        tmp = tmp.replace(Vars.PORT.toString(), Integer.toString(config.getPort()));
        tmp = tmp.replace(Vars.DEBUG.toString(), Integer.toString(config.getDebugLevel()));
        tmp = tmp.replace(Vars.NUMCON.toString(), Integer.toString(master.getNumConnections()));
        tmp = tmp.replace(Vars.NUMREQS.toString(), Long.toString(master.getNumRequests()));
        tmp = tmp.replace(Vars.TIME.toString(), new Date(time).toString() + " (" + time + ")");
        tmp = tmp.replace(Vars.TABLEDUMP.toString(), dump.toString());
        
        tmp = tmp.replace(Vars.VERSION.toString(), VersionManagement.RELEASE_VERSION);
        tmp = tmp.replace(Vars.PROTOVERSION.toString(), Integer.toString(DIRServiceConstants.INTERFACE_ID));
        tmp = tmp.replace(Vars.DBVERSION.toString(), BabuDBFactory.BABUDB_VERSION);
        
        return tmp;
        
    }
}
