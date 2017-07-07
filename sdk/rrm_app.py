
import json
# Make it work for Python 2+3 and with Unicode
import io
import requests
import time
import logging
import argparse
import os
import pprint

import rrm_app_vars

from array import *

#periodic task
from threading import Timer
from time import sleep
from sys import version_info


from lib import flexran_sdk 
from lib import logger

class rrm_app(object):
    """RRM network app to enforce poliy to the underlying RAN
    """
    # stats vars 
    maxmcs_dl= {}
    maxmcs_ul={}
    # consider only one cc
    enb_sfn={}
    enb_ulrb={}
    enb_dlrb={}
    enb_ulmaxmcs={}
    enb_dlmaxmcs={}

    ue_dlwcqi={}
    ue_phr={}

    lc_ue_bsr={}
    lc_ue_report={}


    # performance variables
    enb_available_ulrb={}
    enb_available_dlrb={}
    
    ue_dlmcs={}
    ue_ulmcs={}
   
    lc_ue_dlrb={}
    lc_ue_ulrb={}
    lc_ue_dltbs={}
    lc_ue_ultbs={}
    ue_dlrb={}
    ue_ulrb={}
    ue_dltbs={}
    ue_ultbs={}

    # Slice policy vars
    slice_ulrb = {}
    slice_dlrb = {}
    slice_ulrb_share = {}
    slice_dlrb_share = {}


    def __init__(self, log, url='http://localhost:9999',log_level='info', op_mode='test'):
        super(rrm_app, self).__init__()
        
        self.url = url
        self.log_level = log_level
        self.status = 0
        self.op_mode = op_mode
        
        # RRM App local data
        self.policy_data = {}

        
    def get_statistics(self, sm):
            
        for enb in range(0, sm.get_num_enb()) :
            rrm_app.enb_sfn[enb]  = sm.get_enb_sfn(enb)
            rrm_app.enb_dlrb[enb] = sm.get_cell_rb(enb,dir='DL')
            rrm_app.enb_ulrb[enb] = sm.get_cell_rb(enb,dir='UL')
            rrm_app.enb_ulmaxmcs[enb] = sm.get_cell_maxmcs(enb,dir='UL')
            rrm_app.enb_dlmaxmcs[enb] = sm.get_cell_maxmcs(enb,dir='DL')
            
            for ue in range(0, sm.get_num_ue(enb=enb)) :
                rrm_app.ue_dlwcqi[enb,ue]=sm.get_ue_dlwbcqi(enb,ue)
                rrm_app.ue_phr[enb,ue] =sm.get_ue_phr(enb,ue)
                
                # skip the control channels, SRB1 and SRB2 
                for lc in range(2, sm.get_num_ue_lc(enb=enb,ue=ue)) :
                    # for each lcgid rater than lc
                    rrm_app.lc_ue_bsr[enb,ue,lc] = sm.get_ue_bsr(enb,ue,lc=lc)
                    rrm_app.lc_ue_report[enb, ue, lc] = sm.get_ue_lc_report(enb=enb, ue=ue, lc=lc)

    def get_policy_maxmcs(self,rrm) :
        
        for sid in range(0, rrm.get_num_slices()):
            rrm_app.maxmcs_dl[sid] = rrm.get_slice_maxmcs(sid=sid)
            
        for sid in range(0, rrm.get_num_slices(dir='UL')):
            rrm_app.maxmcs_ul[sid] = rrm.get_slice_maxmcs(sid=sid, dir='UL')
 
    def calculate_exp_perf (self, sm) :

        for enb in range(0, sm.get_num_enb()) :
            rrm_app.enb_available_ulrb[enb]= rrm_app.enb_ulrb[enb]
            rrm_app.enb_available_dlrb[enb]= rrm_app.enb_dlrb[enb]

            for ue in range(0, sm.get_num_ue(enb=enb)) :
                # calculate the MCS
                rrm_app.ue_dlmcs[enb,ue]=rrm_app_vars.cqi_to_mcs[rrm_app.ue_dlwcqi[enb,ue]]
                rrm_app.ue_ulmcs[enb,ue]=8 # f(ue_phr[enb,ue])
            
                # calculate the spectral efficieny
            
                # skip the control channels, SRB1 and SRB2 
                for lc in range(2, sm.get_num_ue_lc(enb=enb,ue=ue)) :
                
                    #calculate the required RB for UL
                    rrm_app.ue_ulrb[enb,ue]=0
                    rrm_app.lc_ue_ulrb[enb,ue,lc]=2
                    ul_itbs=rrm_app_vars.mcs_to_itbs[rrm_app.ue_ulmcs[enb,ue]]
                    rrm_app.lc_ue_ultbs[enb,ue,lc]=rrm_app_vars.tbs_table[ul_itbs][rrm_app.lc_ue_ulrb[enb,ue,lc]]
                    while rrm_app_vars.bsr_table[rrm_app.lc_ue_bsr[enb,ue,lc]] > rrm_app.lc_ue_ultbs[enb,ue,lc] : 
                        if rrm_app.lc_ue_ulrb[enb,ue,lc] > rrm_app.enb_available_ulrb[enb] :
                            log.info('no available ulrb')
                            break
                        rrm_app.lc_ue_ulrb[enb,ue,lc]+=2
                        rrm_app.lc_ue_ultbs[enb,ue,lc]=rrm_app_vars.tbs_table[ul_itbs][rrm_app.lc_ue_ulrb[enb,ue,lc]]
                    
                    rrm_app.ue_ulrb[enb,ue]+=rrm_app.lc_ue_ulrb[enb,ue,lc]
                    rrm_app.enb_available_ulrb[enb]-=rrm_app.ue_ulrb[enb,ue]

                    #calculate the required RB for DL
                    rrm_app.ue_dlrb[enb,ue]=0
                    rrm_app.lc_ue_dlrb[enb,ue,lc]=2
                    dl_itbs=rrm_app_vars.mcs_to_itbs[rrm_app.ue_dlmcs[enb,ue]]
                    rrm_app.lc_ue_dltbs[enb,ue,lc]=rrm_app_vars.tbs_table[dl_itbs][rrm_app.lc_ue_dlrb[enb,ue,lc]]
                    while rrm_app.lc_ue_report[enb, ue, lc]['txQueueSize'] > rrm_app.lc_ue_dltbs[enb,ue,lc] : 
                        if rrm_app.lc_ue_dlrb[enb,ue,lc] > rrm_app.enb_available_dlrb[enb] :
                            log.info('no available dlrb')
                            break
                        rrm_app.lc_ue_dlrb[enb,ue,lc]+=2
                        rrm_app.lc_ue_dltbs[enb,ue,lc]=rrm_app_vars.tbs_table[dl_itbs][rrm_app.lc_ue_dlrb[enb,ue,lc]]

                    rrm_app.ue_dlrb[enb,ue]+=rrm_app.lc_ue_dlrb[enb,ue,lc]
                    rrm_app.enb_available_dlrb[enb]-=rrm_app.ue_dlrb[enb,ue]
                
                    # calculate the total RB for DL and UL
                                
                rrm_app.ue_ultbs[enb,ue]=rrm_app_vars.tbs_table[ul_itbs][rrm_app.ue_ulrb[enb,ue]]
                rrm_app.ue_dltbs[enb,ue]=rrm_app_vars.tbs_table[dl_itbs][rrm_app.ue_dlrb[enb,ue]]
        
                log.info( 'eNB ' + str(enb) + ' UE ' + str(ue) + ' SFN ' + str(rrm_app.enb_sfn[enb]) + 
                          ' DL TBS ' + str(rrm_app.ue_dltbs[enb,ue]) +
                          ' ue_dlrb ' + str(rrm_app.ue_dlrb[enb,ue]) +
                          ' ue_dlmcs ' + str(rrm_app.ue_dlmcs[enb,ue]) +
                          ' --> expected DL throughput ' +  str(float(rrm_app.ue_dltbs[enb,ue]/1000.0)) + ' Mbps')
                
                log.info( 'eNB ' + str(enb) + ' UE ' + str(ue) + ' SFN ' + str(rrm_app.enb_sfn[enb]) + 
                          ' UL TBS ' + str(rrm_app.ue_ultbs[enb,ue])   +
                          ' ue_ulrb ' + str(rrm_app.ue_ulrb[enb,ue])   +
                          ' ue_ulmcs ' + str(rrm_app.ue_ulmcs[enb,ue]) +
                          ' --> expected UL throughput ' +  str(float(rrm_app.ue_ultbs[enb,ue]/1000.0)) + ' Mbps')


    def enforce_policy(self,sm,rrm):

        for enb in range(0, sm.get_num_enb()) :
            for sid in range(0, rrm.get_num_slices()):
                rrm_app.slice_ulrb[enb,sid]=0.0
                rrm_app.slice_dlrb[enb,sid]=0.0
                rrm_app.slice_ulrb_share[enb,sid]=0.0
                rrm_app.slice_dlrb_share[enb,sid]=0.0
             
                for ue in range(0, sm.get_num_ue(enb=enb)) :
                    # simple ue to slice mapping 
                    if ue % rrm_app_vars.max_num_slice == sid :  
                        rrm_app.slice_ulrb[enb,sid]+=rrm_app.ue_ulrb[enb,ue]
                        rrm_app.slice_dlrb[enb,sid]+=rrm_app.ue_dlrb[enb,ue]
                    else :
                        continue 
                rrm_app.slice_ulrb_share[enb,sid]=float(rrm_app.slice_ulrb[enb,sid]/rrm_app.enb_ulrb[enb])
                rrm_app.slice_dlrb_share[enb,sid]=float(rrm_app.slice_dlrb[enb,sid]/rrm_app.enb_dlrb[enb])
                # set the minimum
                if rrm_app.slice_ulrb_share[enb,sid] < 0.1 and rrm_app.slice_ulrb_share[enb,sid] > 0.0:
                    rrm_app.slice_ulrb_share[enb,sid]= 0.1
                if rrm_app.slice_dlrb_share[enb,sid] < 0.1 and rrm_app.slice_dlrb_share[enb,sid] > 0.0:
                    rrm_app.slice_dlrb_share[enb,sid]= 0.1

                log.info( 'eNB ' + str(enb) + ' Slice ' + str(sid) + ' SFN ' + str(rrm_app.enb_sfn[enb]) + 
                        ' slice_ulrb_share: ' + str(rrm_app.slice_ulrb_share[enb,sid]) +
                        ' slice_dlrb_share: ' + str(rrm_app.slice_dlrb_share[enb,sid]) )
                # set the policy files
                rrm.set_slice_rb(sid=sid,rb=rrm_app.slice_ulrb_share[enb,sid], dir='UL')
                rrm.set_slice_rb(sid=sid,rb=rrm_app.slice_dlrb_share[enb,sid], dir='DL')

                rrm.set_slice_maxmcs(sid=sid,maxmcs=min(rrm_app.maxmcs_ul[sid],rrm_app.enb_ulmaxmcs[enb]), dir='UL')
                rrm.set_slice_maxmcs(sid=sid,maxmcs=min(rrm_app.maxmcs_dl[sid],rrm_app.enb_dlmaxmcs[enb]), dir='DL')
                
                rrm.apply_policy()
                rrm.save_policy(time=rrm_app.enb_sfn[enb])
                log.info('_____________Policy enforced______________')
                print rrm.dump_policy()

        
    def run(self, sm,rrm):
        log.info('2. Reading the status of the underlying eNBs')
        sm.stats_manager('all')

        log.info('3. Gather statistics')
        rrm_app.get_policy_maxmcs(rrm)
        rrm_app.get_statistics(sm)
        
        
        log.info('4. Calculate the expected performance')
        rrm_app.calculate_exp_perf(sm)
        
        log.info('5. Check for new RRM Slice policy')
        rrm_app.enforce_policy(sm,rrm)

        t = Timer(10, self.run,kwargs=dict(sm=sm,rrm=rrm))
        t.start()

        
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

    log=flexran_sdk.logger(log_level=args.log).init_logger()
    
    rrm_app = rrm_app(log=log,
                      url=args.url,
                      log_level=args.log,
                      op_mode=args.op_mode)

    rrm = flexran_sdk.rrm_policy(log=log,
                                 url=args.url,
                                 op_mode=args.op_mode)
    policy=rrm.read_policy()
    
    sm = flexran_sdk.stats_manager(log=log,
                                   url=args.url,
                                   op_mode=args.op_mode)
       
    py3_flag = version_info[0] > 2 
    
    t = Timer(3, rrm_app.run,kwargs=dict(sm=sm,rrm=rrm))
    t.start() 

    
    while True:
        # assume the number of slices or slice types (eMBB, uRLLC, mMTC) to be an input
        # prompt the user?
        # predefined slice templates can also be used 
        log.info('1. setting the number of slices')
        

        if py3_flag:
            n = input("Please enter number of slices: ")
        else:
            n = raw_input("Please enter number of slices: ")

        if int(n) < 0 or int(n) > 4 :
            log.error('wrong number of slices')
        
        rrm.set_num_slices(n=int(n), dir='DL')
        rrm.set_num_slices(n=int(n), dir='UL')
        
        sleep(10)
