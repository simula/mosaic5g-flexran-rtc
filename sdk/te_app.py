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
    File name: te_app.py
    Author: navid nikaein
    Description: This app triggers an external event based on the predefined threshold through FLEXRAN SDK
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

class te_app(object):
    """trigger external events happend in the underlying RAN
    """
    # stats vars
    enb_sfn={}
    enb_ulrb={}
    enb_dlrb={}
    enb_ulmaxmcs={}
    enb_dlmaxmcs={}
    enb_dlwcqi_min={}

    ue_dlwcqi={}
    ue_dlwcqi_avg={}
    
    ue_phr={}

    lc_ue_bsr={}
    lc_ue_report={}


    def __init__(self, log, url='http://localhost',port='9999',url_app='http://localhost',port_app='9090', log_level='info', op_mode='test'):
        super(te_app, self).__init__()
        
        self.url = url+':'+port
        self.log = log
        self.log_level = log_level
        self.status = 'none'
        self.op_mode = op_mode
        
        # server vars
        self.url_app=url_app+':'+port_app
        self.alpha= 0.7
                   
    def get_statistics(self, sm):
            
        for enb in range(0, sm.get_num_enb()) :
	    te_app.enb_dlrb[enb] = sm.get_cell_rb(enb,dir='DL')
            te_app.enb_ulrb[enb] = sm.get_cell_rb(enb,dir='UL')
            te_app.enb_ulmaxmcs[enb] = sm.get_cell_maxmcs(enb,dir='UL')
            te_app.enb_dlmaxmcs[enb] = sm.get_cell_maxmcs(enb,dir='DL')
            te_app.enb_dlwcqi_min[enb]=15
            
            for ue in range(0, sm.get_num_ue(enb=enb)) :
          	te_app.enb_sfn[enb,ue]  = sm.get_enb_sfn(enb,ue)
		te_app.ue_dlwcqi[enb,ue]=sm.get_ue_dlwbcqi(enb,ue)
                #te_app.ue_dlwcqi_avg[enb,ue]=self.alpha*te_app.ue_dlwcqi_avg[enb,ue] + (1-self.alpha)*te_app.ue_dlwcqi[enb,ue]
                te_app.ue_phr[enb,ue] =sm.get_ue_phr(enb,ue)
                
                te_app.enb_dlwcqi_min[enb]=min(te_app.enb_dlwcqi_min[enb], te_app.ue_dlwcqi[enb,ue])
                
                # skip the control channels, SRB1 and SRB2 
                for lc in range(2, sm.get_num_ue_lc(enb=enb,ue=ue)) :
                    # for each lcgid rater than lc
                    te_app.lc_ue_bsr[enb,ue,lc] = sm.get_ue_bsr(enb,ue,lc=lc)
                    te_app.lc_ue_report[enb, ue, lc] = sm.get_ue_lc_report(enb=enb, ue=ue, lc=lc)

 
    def get_link_quality(self,sm):
         for enb in range(0, sm.get_num_enb()) :
             if te_app.enb_dlwcqi_min[enb] > 10 :
                 return 'high'
             elif te_app.enb_dlwcqi_min[enb] > 6 :
                 return 'medium'
             else :
                 return 'low'
        
    def check_events(self, sm, event='cqi'):
        data =''
        if event == 'cqi' or event == 'CQI' : 
            data='congestion=0&bitrate='+self.get_link_quality(sm)
            self.post_event(data)
        else:
            self.log.warning('undefined trigger event ' + event)

    def post_event(self, data):
        
        if self.op_mode == 'test' :
            self.log.info('POST ' + self.url_app)
            self.log.info('Data ' + str(data))
        
        elif self.op_mode == 'sdk' : 
            try :
                self.log.debug('POST ' + self.url_app)
                self.log.debug('Data ' + data)
                req = requests.post(url_app,str(data),headers={'Content-Type': 'application/x-www-form-urlencoded'})
                if req.status_code == 200 :
                    self.log.error('successfully post the event' )
                    self.status='connected'
                else :
                    self.status='disconnected'
                self.log.error('Request error code : ' + req.status_code)
            except :
                self.log.error('Failed to post the event' )

        else :
            self.log.warn('Unknown operation mode ' + op_mode )       

    
    def run(self, sm):
        log.info('2. Reading the status of the underlying eNBs')
        sm.stats_manager('all')

        log.info('3. Gather statistics')
        te_app.get_statistics(sm)
        
        log.info('4. Check events')
        te_app.check_events(sm)
        
        
        t = Timer(5, self.run,kwargs=dict(sm=sm))
        t.start()
        
        
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    
    parser.add_argument('--url', metavar='[option]', action='store', type=str,
                        required=False, default='http://localhost', 
                        help='set the FlexRAN RTC URL: loalhost (default)')
    parser.add_argument('--port', metavar='[option]', action='store', type=str,
                        required=False, default='9999', 
                        help='set the FlexRAN RTC port: 9999 (default)')
    parser.add_argument('--url-app', metavar='[option]', action='store', type=str,
                        required=False, default='http://localhost', 
                        help='set the application server URL: loalhost (default)')
    parser.add_argument('--port-app', metavar='[option]', action='store', type=str,
                        required=False, default='9090', 
                        help='set the application server port: 9999 (default)')
    parser.add_argument('--op-mode', metavar='[option]', action='store', type=str,
                        required=False, default='sdk', 
                        help='Set the app operation mode either with FlexRAN or with the test json files: test, sdk(default)')
    parser.add_argument('--log',  metavar='[level]', action='store', type=str,
                        required=False, default='info', 
                        help='set the log level: debug, info (default), warning, error, critical')
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')

    args = parser.parse_args()

    log=flexran_sdk.logger(log_level=args.log).init_logger()
    
    te_app = te_app(log=log,
                    url=args.url,
                    port=args.port,
                    url_app=args.url_app,
                    port_app=args.port_app,
                    log_level=args.log,
                    op_mode=args.op_mode)

    sm = flexran_sdk.stats_manager(log=log,
                                   url=args.url,
                                   port=args.port,
                                   op_mode=args.op_mode)
       
    py3_flag = version_info[0] > 2 
    
    t = Timer(3, te_app.run,kwargs=dict(sm=sm))
    t.start() 

            
