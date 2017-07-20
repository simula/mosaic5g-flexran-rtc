'''
   The MIT License (MIT)

   Copyright (c) 2017

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
'''

'''
    File name: rrm_app.py
    Author: navid nikaein
    Description: This app dynamically updates the RRM policy based on the statistics received through FLEXRAN SDK
    version: 1.0
    Date created: 7 July 2017
    Date last modified: 7 July 2017 
    Python Version: 2.7
    
'''



import json
# Make it work for Python 2+3 and with Unicode
import io
import requests
import time
import logging
import argparse
import os
import pprint
import sys 
from sys import *

from array import *
from threading import Timer
from time import sleep

import rrm_app_vars

from lib import flexran_sdk 
from lib import logger

import signal

def sigint_handler(signum,frame):
    print 'Exiting, wait for the timer to expire... Bye'
    sys.exit(0)

signal.signal(signal.SIGINT, sigint_handler)

class rrm_app(object):
    """RRM network app to enforce poliy to the underlying RAN
    """
    #
    nb_slice=1
    nb_slice_current=0
    #
    pf=''
    pf_current=''
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
    enb_ulrb_share = {}
    enb_dlrb_share = {}


    def __init__(self, log, template=rrm_app_vars.template_1, url='http://localhost',port='9999',log_level='info', op_mode='test'):
        super(rrm_app, self).__init__()
        
        self.template=template
        self.url = url+port
        self.log_level = log_level
        self.status = 0
        self.op_mode = op_mode
               
        # RRM App local data
        self.policy_data = {}

	# TBD: init for max enb and ue 
	rrm_app.enb_sfn[0,0]={0}
        
    def get_statistics(self, sm):
            
        for enb in range(0, sm.get_num_enb()) :
	    rrm_app.enb_dlrb[enb] = sm.get_cell_rb(enb,dir='DL')
            rrm_app.enb_ulrb[enb] = sm.get_cell_rb(enb,dir='UL')
            rrm_app.enb_ulmaxmcs[enb] = sm.get_cell_maxmcs(enb,dir='UL')
            rrm_app.enb_dlmaxmcs[enb] = sm.get_cell_maxmcs(enb,dir='DL')
            
            for ue in range(0, sm.get_num_ue(enb=enb)) :
          	rrm_app.enb_sfn[enb,ue]  = sm.get_enb_sfn(enb,ue)
		rrm_app.ue_dlwcqi[enb,ue]=sm.get_ue_dlwbcqi(enb,ue)
                rrm_app.ue_phr[enb,ue] =sm.get_ue_phr(enb,ue)
                
                # skip the control channels, SRB1 and SRB2 
                for lc in range(2, sm.get_num_ue_lc(enb=enb,ue=ue)) :
                    # for each lcgid rater than lc
                    rrm_app.lc_ue_bsr[enb,ue,lc] = sm.get_ue_bsr(enb,ue,lc=lc)
                    rrm_app.lc_ue_report[enb, ue, lc] = sm.get_ue_lc_report(enb=enb, ue=ue, lc=lc)

    def get_policy_maxmcs(self,rrm) :
        
        #for sid in range(0, rrm.get_num_slices()):
        for sid in range(0, rrm_app_vars.max_num_slice):
            rrm_app.maxmcs_dl[sid] = rrm.get_slice_maxmcs(sid=sid)
            
        #for sid in range(0, rrm.get_num_slices(dir='UL')):
        for sid in range(0, rrm_app_vars.max_num_slice):
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
        
                log.info( 'eNB ' + str(enb) + ' UE ' + str(ue) + ' SFN ' + str(rrm_app.enb_sfn[enb,ue]) + 
                          ' DL TBS ' + str(rrm_app.ue_dltbs[enb,ue]) +
                          ' ue_dlrb ' + str(rrm_app.ue_dlrb[enb,ue]) +
                          ' ue_dlmcs ' + str(rrm_app.ue_dlmcs[enb,ue]) +
                          ' --> expected DL throughput ' +  str(float(rrm_app.ue_dltbs[enb,ue]/1000.0)) + ' Mbps')
                
                log.info( 'eNB ' + str(enb) + ' UE ' + str(ue) + ' SFN ' + str(rrm_app.enb_sfn[enb,ue]) + 
                          ' UL TBS ' + str(rrm_app.ue_ultbs[enb,ue])   +
                          ' ue_ulrb ' + str(rrm_app.ue_ulrb[enb,ue])   +
                          ' ue_ulmcs ' + str(rrm_app.ue_ulmcs[enb,ue]) +
                          ' --> expected UL throughput ' +  str(float(rrm_app.ue_ultbs[enb,ue]/1000.0)) + ' Mbps')


    def determine_rb_share(self,sm,rrm):

        for enb in range(0, sm.get_num_enb()) :

            rrm_app.enb_ulrb_share[enb]=0.0
            rrm_app.enb_dlrb_share[enb]=0.0
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

	        log.debug( 'S1: eNB ' + str(enb) + ' Slice ' + str(sid) + ' SFN ' + str(rrm_app.enb_sfn[enb,0]) + 
                          ' slice_ulrb_share: ' + str(rrm_app.slice_ulrb_share[enb,sid]) +
                          ' slice_dlrb_share: ' + str(rrm_app.slice_dlrb_share[enb,sid]) )

                rrm_app.enb_ulrb_share[enb]+=rrm_app.slice_ulrb_share[enb,sid]
                rrm_app.enb_dlrb_share[enb]+=rrm_app.slice_dlrb_share[enb,sid]
            
            # disribute the remaining rb at the second stage
            # TODO: allocate based on SLA
            extra_ul=((1.0 - rrm_app.enb_ulrb_share[enb])/rrm.get_num_slices())
            extra_dl=((1.0 - rrm_app.enb_dlrb_share[enb])/rrm.get_num_slices())
            for sid in range(0, rrm.get_num_slices()):

                if  extra_ul > 0 :
                    rrm_app.slice_ulrb_share[enb,sid]+=extra_ul
                    rrm_app.enb_ulrb_share[enb]+=extra_ul
                    
                if  extra_dl > 0 :
                    rrm_app.slice_dlrb_share[enb,sid]+=extra_dl
                    rrm_app.enb_dlrb_share[enb]+=extra_dl
                    
                    
                log.debug( 'S2: eNB ' + str(enb) + ' Slice ' + str(sid) + ' SFN ' + str(rrm_app.enb_sfn[enb,0]) + 
                          ' slice_ulrb_share: ' + str(rrm_app.slice_ulrb_share[enb,sid]) +
                          ' slice_dlrb_share: ' + str(rrm_app.slice_dlrb_share[enb,sid]) )

                log.info( 'eNB ' + str(enb) + ' Slice ' + str(sid) + ' SFN ' + str(rrm_app.enb_sfn[enb,0]) + 
                      ' ulrb_share: ' + str(rrm_app.enb_ulrb_share[enb]) +
                      ' dlrb_share: ' + str(rrm_app.enb_dlrb_share[enb]) )
            
                                
            
    def enforce_policy(self,sm,rrm):

        for enb in range(0, sm.get_num_enb()) :
            for sid in range(0, rrm.get_num_slices()):
                
                # set the policy files
                rrm.set_slice_rb(sid=sid,rb=rrm_app.slice_ulrb_share[enb,sid], dir='UL')
                rrm.set_slice_rb(sid=sid,rb=rrm_app.slice_ulrb_share[enb,sid], dir='DL')
                rrm.set_slice_maxmcs(sid=sid,maxmcs=min(rrm_app.maxmcs_ul[sid],rrm_app.enb_ulmaxmcs[enb]), dir='UL')
                rrm.set_slice_maxmcs(sid=sid,maxmcs=min(rrm_app.maxmcs_dl[sid],rrm_app.enb_dlmaxmcs[enb]), dir='DL')

                # ToDO: check if we should push sth
            if sm.get_num_ue(enb) > 0 : 
	        if rrm.apply_policy() == 'connected' :
		    rrm_app.pf=rrm.save_policy(time=rrm_app.enb_sfn[enb,0])
		    log.info('_____________eNB' + str(enb)+' enforced policy______________')
		    print rrm.dump_policy()
                    
	    else: 
		log.info('No UE is attached yet')

        
    def run(self, sm,rrm):
        log.info('2. Reading the status of the underlying eNBs')
        sm.stats_manager('all')

        log.info('3. Gather statistics')
        rrm_app.get_policy_maxmcs(rrm)
        rrm_app.get_statistics(sm)
        
        
        log.info('4. Calculate the expected performance')
        rrm_app.calculate_exp_perf(sm)
        
        log.info('5. Determine RB share per slice')
        rrm_app.determine_rb_share(sm,rrm)

        log.info('6. Check for new RRM Slice policy')
        rrm_app.enforce_policy(sm,rrm)
        
        t = Timer(5, self.run,kwargs=dict(sm=sm,rrm=rrm))
        t.start()
        
        
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    
    parser.add_argument('--url', metavar='[option]', action='store', type=str,
                        required=False, default='http://localhost', 
                        help='set the FlexRAN RTC URL: loalhost (default)')
    parser.add_argument('--port', metavar='[option]', action='store', type=str,
                        required=False, default='9999', 
                        help='set the FlexRAN RTC port: 9999 (default)')
    parser.add_argument('--template', metavar='[option]', action='store', type=str,
                        required=False, default='template_1', 
                        help='set the slice template to indicate the type of each slice: template_1(default), .... template_4')
    parser.add_argument('--op-mode', metavar='[option]', action='store', type=str,
                        required=False, default='sdk', 
                        help='Set the app operation mode either with FlexRAN or with the test json files: test, sdk(default)')
    parser.add_argument('--log',  metavar='[level]', action='store', type=str,
                        required=False, default='info', 
                        help='set the log level: debug, info (default), warning, error, critical')
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')

    args = parser.parse_args()

    log=flexran_sdk.logger(log_level=args.log).init_logger()
    
    rrm_app = rrm_app(log=log,
                      template=args.template,
                      url=args.url,
                      port=args.port,
                      log_level=args.log,
                      op_mode=args.op_mode)

    rrm = flexran_sdk.rrm_policy(log=log,
                                 url=args.url,
                                 port=args.port,
                                 op_mode=args.op_mode)
    policy=rrm.read_policy()
    
    sm = flexran_sdk.stats_manager(log=log,
                                   url=args.url,
                                   port=args.port,
                                   op_mode=args.op_mode)
       
    py3_flag = version_info[0] > 2 
    
    t = Timer(3, rrm_app.run,kwargs=dict(sm=sm,rrm=rrm))
    t.start() 

    
    while True:

        try:
       # assume the number of slices or slice types (eMBB, uRLLC, mMTC) to be an input
       # prompt the user?
       # predefined slice templates can also be used 
         
            if py3_flag:
                rrm_app.nb_slice = int(input("Update the number of slices: "))
            else:
                rrm_app.nb_slice = int(raw_input("Update the number of slices: "))
                
        except ValueError:
            log.warning('Please entre an integer between 1-4')
            
        if rrm_app.nb_slice < 0 or rrm_app.nb_slice > 4 :
            log.warning('wrong number of slices + ' + str(rrm_app.nb_slice) + '!')
            log.warning('Please entre an integer between 1-4')
        else:   
            rrm.set_num_slices(n=int(rrm_app.nb_slice), dir='DL')
            rrm.set_num_slices(n=int(rrm_app.nb_slice), dir='UL')

        if rrm_app.nb_slice != rrm_app.nb_slice_current :
            log.info('Number of slices is set to ' + str(rrm_app.nb_slice))
            rrm_app.nb_slice_current = rrm_app.nb_slice
            
        sleep(5)
#except KeyboardInterrupt:
#            print "Exiting : Wait for the timer to expires... Bye"
#            sys.exit(0)
        
