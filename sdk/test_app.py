
import json
# Make it work for Python 2+3 and with Unicode
import io
import requests
import time
import logging
import argparse
import os
import pprint

from flexran_sdk import *
from logger import *

class test_app(object):
    """test flexran app that makes use of flexran SDK
    """

    def __init__(self, url='http://localhost:9999',log_level='info', op_mode='test'):
        super(test_app, self).__init__()
        
        self.url = url
        self.log_level = log_level
        self.status = 0
        self.op_mode = op_mode
        
        # NB APIs endpoints
        #self.all_stats_api='/stats_manager/json/all'
        #self.enb_stats_api='/stats_manager/json/enb_config'
        #self.mac_stats_api='/stats_manager/json/mac_stats'
        #self.rrm_api='/rrm'
        #self.dl_sched_api='/dl_sched'
        #self.rrc_stats_api='/rrc_trigger'

            
   
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    
    parser.add_argument('--url', metavar='[option]', action='store', type=str,
                        required=False, default='http://localhost:9999', 
                        help='set the FlexRAN RTC URL: loalhost (default)')
    parser.add_argument('--op-mode', metavar='[option]', action='store', type=str,
                        required=False, default='test', 
                        help='Test SDK with already generated json files: test (default), sdk')
    parser.add_argument('--log',  metavar='[level]', action='store', type=str,
                        required=False, default='info', 
                        help='set the log level: debug, info (default), warning, error, critical')
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')

    args = parser.parse_args()

    app = test_app(url=args.url,
                   log_level=args.log,
                   op_mode=args.op_mode)

    log=logger(log_level=args.log).init_logger()
    log.info('test')
    
    sm = stats_manager(log=log,
                       url=args.url,
                       op_mode=args.op_mode)
    sm.stats_manager('all')
    if args.op_mode == 'test' :
        print 'enb'
        print json.dumps(sm.get_enb_config(), indent=2)
        print 'ue'
        print json.dumps(sm.get_ue_config(), indent=2)
        print 'cell'
        print json.dumps(sm.get_cell_config(), indent=2)
        print 'lc'
        print json.dumps(sm.get_lc_config(), indent=2)
        print 'ue status'
        print json.dumps(sm.get_ue_status(), indent=2)
        print 'ue mac status'
        print json.dumps(sm.get_ue_mac_status(), indent=2)
        print 'phr'
        print json.dumps(sm.get_ue_phr(), indent=2)
        print 'rlc report'
        print json.dumps(sm.get_ue_rlc_report(), indent=2)
        print 'DL CQI report'
        print json.dumps(sm.get_ue_dlcqi_report(), indent=2)
        print 'harq'
        print json.dumps(sm.get_ue_harq(), indent=2)
        print 'SFN'
        print json.dumps(sm.get_enb_sfn(), indent=2)

        rrm = rrm(log=log,
                  url=args.url,
                  op_mode=args.op_mode)

        rrm.apply_policy()
        rrm.save_policy()

        print 'num dl slices are: ' + str(rrm.get_num_slices())
        rrm.set_num_slices(5)
        rrm.dump_policy()
        rrm.set_num_slices(3,'ul')
        print 'num dl slices are: ' + str(rrm.get_num_slices())

        rrm.set_slice_rb(0, 0.3)
        rrm.set_slice_rb(2, 0.7)
        rrm.dump_policy()
        rrm.save_policy()
        print 'get rb percentage for slice 2: ' + str(rrm.get_slice_rb(2))

        rrm.set_slice_maxmcs(0, 10)
        rrm.set_slice_maxmcs(1, 16)
        rrm.dump_policy()
        print 'get max mcs for slice 1: ' + str(rrm.get_slice_maxmcs(1))

        #ac = cd_action()
        #print cd_action.PULL
        #print cd_action(0).PULL #.describe()
        #print member.value

        cd=control_delegate(log=log,
                            url=args.url,
                            op_mode=args.op_mode)
        cd.delegate_agent(action="PULL")


        rrc_tm=rrc_trigger_meas(log=log,
                                url=args.url,
                                op_mode=args.op_mode)

        rrc_tm.trigger_meas()
