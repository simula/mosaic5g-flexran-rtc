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
    File name: flexran_sdk.py
    Author: navid nikaein
    Description: Library to get and set accessible paramters and operation through the FlexRAN RTC
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
import yaml

from logger import *

from enum import Enum


class rrc_triggers(Enum):

    ONE_SHOT = 0
    PERIODIC = 1
    EVENT_DRIVEN= 2

class cd_actions(Enum):

    PULL = 0
    PUSH = 1
    
    #def describe(self):
        #return self.name, self.value

    #def __str__(self):
        #return '%s' % self._value_

class flexran_rest_api(object):

    # policy file
    pf_all='outputs/all_2.json'
    pf_mac='outputs/mac_stats_2.json'
    pf_enb='outputs/enb_config_2.json'

    # relateive to flexran apps
    pf_name='enb_dl_scheduling_policy.yaml'
    pf_yaml='../tests/delegation_control/'+pf_name
    pf_json='{"mac": [{"dl_scheduler": {"parameters": {"n_active_slices": 1,"slice_percentage": [1,0.4,0,0],"slice_maxmcs": [28,28,28,28 ]}}}]}'

    # APIs 
    rrc_trigger='/rrc_trigger'

    cd_dl='/dl_sched'
    cd_ul='/ul_sched'

    rrm='/rrm'  
    rrm_policy='/rrm_config'  

    sm_all='/stats_manager/json/all'
    sm_enb='/stats_manager/json/enb_config'
    sm_mac='/stats_manager/json/mac_stats'
        
class rrc_trigger_meas(object):
    def __init__(self, log, url='http://localhost', port='9999', op_mode='test'):
        super(rrc_trigger_meas, self).__init__()
        
        self.url = url+':'+port
        self.status = ''
        self.op_mode = op_mode
        self.log = log

        self.rrc_trigger_api=flexran_rest_api.rrc_trigger

        self.rrc_meas = {}
        self.rrc_meas[rrc_triggers.ONE_SHOT]      = 'ONE_SHOT'
        self.rrc_meas[rrc_triggers.PERIODIC]       = 'PERIODIC'
        self.rrc_meas[rrc_triggers.EVENT_DRIVEN]  = 'EVENT_DRIVEN'
        

    def trigger_meas(self, type='PERIODIC'):

        if type == self.rrc_meas[rrc_triggers.ONE_SHOT] :
            url = self.url+self.rrc_trigger_api+'/'+type.lower()
        elif type == self.rrc_meas[rrc_triggers.PERIODIC] :
            url = self.url+self.rrc_trigger_api+'/'+type.lower()
        elif type == self.rrc_meas[rrc_triggers.EVENT_DRIVEN] :
            url = self.url+self.rrc_trigger_api+'/'+type.lower()
        else:
            self.log.error('Type ' + type + 'not supported')
            return
        
        if self.op_mode == 'test' :
            self.log.info('POST ' + str(url))

        elif self.op_mode == 'sdk' : 
            try :
                req = requests.post(url)
                if req.status_code == 200 :
                    self.log.error('successfully delegated the dl scheduling to the agent' )
                    self.status='connected'
                else :
                    self.status='disconnected'
                self.log.error('Request error code : ' + req.status_code)
            except :
                self.log.error('Failed to delegate the DL schedling to the agent' )

        else :
            self.log.warn('Unknown operation mode ' + op_mode )       

class control_delegate(object):
    def __init__(self, log, url='http://localhost', port='9999', op_mode='test'):
        super(control_delegate, self).__init__()

        self.url = url+':'+port
        self.status = ''
        self.op_mode = op_mode
        self.log = log
        # NB APIs endpoints
        self.cd_dl_api=flexran_rest_api.cd_dl
        self.cd_ul_api=flexran_rest_api.cd_ul
        #self.cd_actions[cd_actions.PULL]='PULL'
        #self.cd_actions[cd_actions.PUSH]='PUSH'
        self.actions = {}
        self.actions[cd_actions.PULL] = 'PULL'
        self.actions[cd_actions.PUSH] = 'PUSH'
        
    def delegate_agent(self, func='dl_sched', action='PUSH'):
        if func == 'dl_sched' : 
            url = self.url+self.cd_dl_api
        elif func == 'ul_sched' :
            url = self.url+self.cd_ul_api
        else :
            self.log.error('Func ' + fucn + 'not supported')
            return 

        if action == self.actions[cd_actions.PULL] :
            self.log.debug('Action: Pull ' + func + 'to the controller')
            url = url + '/0'
        elif action == self.actions[cd_actions.PUSH] :
            self.log.debug('Action: Push/delegate ' + func + 'to the agent' )
            url = url + '/1'

        if self.op_mode == 'test' :
            self.log.info('POST ' + str(url))

        elif self.op_mode == 'sdk' : 
            try :
                req = requests.post(url)
                if req.status_code == 200 :
                    self.log.error('successfully delegated the dl scheduling to the agent' )
                    self.status='connected'
                else :
                    self.status='disconnected'
                self.log.error('Request error code : ' + req.status_code)
            except :
                self.log.error('Failed to delegate the DL schedling to the agent' )

        else :
            self.log.warn('Unknown operation mode ' + op_mode )                 
        
class rrm_policy (object):
    def __init__(self, log, url='http://localhost', port='9999', op_mode='test'):
        super(rrm_policy, self).__init__()
        
        self.url = url+':'+port
        self.status = 0
        self.op_mode = op_mode
        self.log = log
        # NB APIs endpoints
        self.rrm_api=flexran_rest_api.rrm
        self.rrm_policy_api=flexran_rest_api.rrm_policy
        # stats manager data requeted by the endpoint
        # could be extended to have data per API endpoint
        self.policy_data = ''

        # test files
        # location must be reletaive to app and not SDK
        # To do: create env vars 
        self.pf_yaml=flexran_rest_api.pf_yaml
        self.pf_json=flexran_rest_api.pf_json

    # read the policy file     
    def read_policy(self, pf=''):

        if os.path.isfile(pf) :
            pfile=pf
        elif os.path.isfile(self.pf_yaml) :
            pfile=self.pf_yaml
        else :
            self.log.error('cannot find the policy file '  + self.pf_yaml + ' or ' + pf)
            return

                
        try:
            with open(pfile) as data_file:
                self.policy_data = yaml.load(data_file)
                self.log.debug(yaml.dump(self.policy_data, default_flow_style=False))
        except yaml.YAMLError, exc:
            self.log.error('error in policy file'  + pfile + str(exc) )
	    return	

	self.print_policy() 
   
        return self.policy_data

    # apply policy with policy data 
    # TBD: apply policy from a file
    def apply_policy(self, policy_data='',as_payload=True):

        self.status=''

        if policy_data != '' :
            pdata=policy_data
        elif self.policy_data != '' :
            pdata=self.policy_data 
        else:
            self.log.error('cannot find the policy data '  + pdata)
            return
	
        if as_payload == True :
            url = self.url+self.rrm_policy_api
        else: 
            url = self.url+self.rrm_api+'/'+flexran_rest_api.pf_name
        
        if self.op_mode == 'test' :
            self.log.info('POST ' + str(url))
            self.log.debug(self.dump_policy(policy_data=pdata))
            self.status='connected'
            
        elif self.op_mode == 'sdk' :
            print self.dump_policy(pdata)
            try :
		# post data as binary
            	req = requests.post(url, data=self.dump_policy(pdata),headers={'Content-Type': 'application/octet-stream'})
		
            	if req.status_code == 200:
            	    self.log.info('successfully applied the policy')
            	    self.status='connected'
            	else :
            	    self.status='disconnected'
            	    self.log.error('Request error code : ' + req.status_code)
            except :
                self.log.error('Failed to apply the policy ' )
            
        else :
            self.log.warn('Unknown operation mode ' + op_mode )
	    self.status='unknown'
        return self.status 

    def dump_policy(self, policy_data=''):
        
        if policy_data != '' :
            return yaml.dump(policy_data, default_flow_style=False, default_style='"')
        elif self.policy_data != '' :
            return yaml.dump(self.policy_data, default_flow_style=False, default_style='"')
        else:
            self.log.error('cannot find the policy data ' + policy_data)
            return

    def print_policy(self):
        print self.dump_policy()

    def save_policy(self, basedir='./', basefn='policy', time=0, format='yaml'):

        fn = os.path.join(basedir,basefn + '_'+str(time) + "." + format)
        #stream = file('policy.yaml', 'w')
        stream = file(fn, 'w')

        if format == 'yaml' or format == 'YAML':
            yaml.dump(self.policy_data, stream)
        elif format == 'json' :
            json.dump(self.policy_data, stream)
        else :
            self.log.error('unsupported format')
            
        return self.policy_data
            
    def set_num_slices(self, n, dir='dl'):
        if dir == 'dl' or dir == "DL":
            index = 0
            key = 'dl_scheduler'
        elif dir == 'ul' or dir == "UL":
            index = 1
            key = 'ul_scheduler'
            
        self.log.debug('Setting the number of ' + dir + ' slices from ' + str(self.policy_data['mac'][index][key]['parameters']['n_active_slices']) + ' to ' + str(n) )
        self.policy_data['mac'][index][key]['parameters']['n_active_slices']=n

    def get_num_slices(self, dir='dl'):
        if dir == 'dl' or dir == "DL":
            index = 0
            key = 'dl_scheduler'
        elif dir == 'ul' or dir == "UL":
            index = 1
            key = 'ul_scheduler'
            
        return  self.policy_data['mac'][index][key]['parameters']['n_active_slices']
        

    def set_slice_rb(self, sid, rb, dir='dl'):
        if dir == 'dl' or dir == "DL":
            index = 0
            key = 'dl_scheduler'
        elif dir == 'ul' or dir == "UL":
            index = 1
            key = 'ul_scheduler'
        else :
            self.log.error('Unknown direction ' + dir)
            return
            
        if sid < 0 or sid > 4 :
            self.log.error('Out of Range slice id')
            return

        if rb < 0 or rb > 1 : 
            self.log.error('Out of Range RB percentage')
            return
           
        self.log.debug('Setting ' + dir + ' slice ' + str(sid) + ' RB from ' + str(self.policy_data['mac'][index][key]['parameters']['slice_percentage'][sid]) + ' to ' + str(rb) )
        self.policy_data['mac'][index][key]['parameters']['slice_percentage'][sid]=rb
        #print self.policy_data['mac'][index][key]['parameters']['slice_percentage'][sid]

    def get_slice_rb(self, sid, dir='dl'):
        if dir == 'dl' or dir == "DL":
            index = 0
            key = 'dl_scheduler'
        elif dir == 'ul' or dir == "UL":
            index = 1
            key = 'ul_scheduler'
        else :
            self.log.error('Unknown direction ' + dir)
            return
            
        if sid < 0 or sid > 4 :
            self.log.error('Out of Range slice id')
            return
 
        return self.policy_data['mac'][index][key]['parameters']['slice_percentage'][sid]

    def set_slice_maxmcs(self, sid, maxmcs=28, dir='dl'):

        if dir == 'dl' or dir == "DL":
            index = 0
            key = 'dl_scheduler'
            mcs=min(maxmcs,28)
        elif dir == 'ul' or dir == "UL":
            index = 1
            key = 'ul_scheduler'
            # use get_cell_maxmcs(enb) from sm
            mcs = min(maxmcs,16)
        else :
            self.log.error('Unknown direction ' + dir)
            return
            
 
        self.log.debug('Setting ' + dir + ' slice ' + str(sid) + ' MCS from ' + str(self.policy_data['mac'][index][key]['parameters']['slice_maxmcs'][sid]) + ' to ' + str(mcs))
        self.policy_data['mac'][index][key]['parameters']['slice_maxmcs'][sid]=mcs
        #print self.policy_data['mac'][index][key]['parameters']['slice_maxmcs'][sid]

    def get_slice_maxmcs(self, sid, dir='dl'):
        if dir == 'dl' or dir == "DL":
            index = 0
            key = 'dl_scheduler'
        elif dir == 'ul' or dir == "UL":
            index = 1
            key = 'ul_scheduler'
        else :
            self.log.error('Unknown direction ' + dir)
            return
            
        if sid < 0 or sid > 4 :
            self.log.error('Out of Range slice id')
            return
 
        return self.policy_data['mac'][index][key]['parameters']['slice_maxmcs'][sid]

    
class stats_manager(object):
    def __init__(self, log, url='http://localhost', port='9999', op_mode='test'):
        super(stats_manager, self).__init__()
        
        self.url = url+':'+port
        self.status = 0
        self.op_mode = op_mode
        self.log = log
        # NB APIs endpoints
        self.sm_all_api=flexran_rest_api.sm_all
        self.sm_enb_api=flexran_rest_api.sm_enb
        self.sm_mac_api=flexran_rest_api.sm_mac

        # stats manager data requeted by the endpoint
        # could be extended to have data per API endpoint
        self.stats_data = ''
        
        # test files
        self.pfile_all=flexran_rest_api.pf_all
        self.pfile_mac=flexran_rest_api.pf_mac
        self.pfile_enb=flexran_rest_api.pf_enb
        

    def stats_manager(self, api):
        self.log.debug('set stats_manager API to :' + str(api))
        file = ''
        #url = '' 
        if self.op_mode == 'test' :
            
            if 'all' in api :
                file =  self.pfile_all
            elif 'mac' in api :
                file =  self.pfile_mac
            elif 'enb' in api :
                file =  self.pfile_enb
            
            try:
                with open(file) as data_file:
                    self.stats_data = json.load(data_file)
                    self.status='connected'
            except :
                self.status='disconnected'
                self.log.error('cannot find the output file'  + file )       

        elif self.op_mode == 'sdk' :

            if 'all' in api :
                url = self.url+self.sm_all_api
            elif 'mac' in api :
                url = self.url+self.sm_mac_api
            elif 'enb' in api :
                url = self.url+self.sm_enb_api
            
            
            self.log.info('the request url is: ' + url)
            try :
                req = requests.get(url)
                if req.status_code == 200:
                    self.stats_data = req.json()
                    self.status='connected'
                else :
                    self.status='disconnected'
                    self.log.error('Request error code : ' + req.status_code)
            except :
                self.log.error('Request url ' + url + ' failed')
            
        else :
            self.log.warn('Unknown operation mode ' + op_mode )
            
        if self.status == 'connected' :     
            self.log.debug('Stats Manager requested data')
            self.log.debug(json.dumps(self.stats_data, indent=2))
                
    def get_enb_config(self,enb=0):
        return self.stats_data['eNB_config'][enb]

    def get_num_enb(self):
        return len(self.stats_data['eNB_config'])

    def get_ue_config(self,enb=0,ue=0):
        return self.stats_data['eNB_config'][enb]['UE']['ueConfig'][ue]

    def get_cell_config(self,enb=0,cc=0):
        return self.stats_data['eNB_config'][enb]['eNB']['cellConfig'][cc]

    def get_cell_rb(self,enb=0,cc=0, dir='dl'):
        if dir == 'dl' or dir == 'DL' :
            return self.stats_data['eNB_config'][enb]['eNB']['cellConfig'][cc]['dlBandwidth']
        elif dir == 'ul' or dir == 'UL' :
            return self.stats_data['eNB_config'][enb]['eNB']['cellConfig'][cc]['ulBandwidth']
        else :
            self.log.warning('unknown direction ' + dir + 'set to DL')
            return self.stats_data['eNB_config'][enb]['eNB']['cellConfig'][cc]['dlBandwidth']

    def get_cell_maxmcs(self,enb=0,cc=0, dir='dl'):

        if dir == 'dl' or dir == 'DL' :
            return 28
        elif self.stats_data['eNB_config'][enb]['eNB']['cellConfig'][cc]['enable64QAM'] == 0 :
            return 16
        else :
            return 28
    
    def get_lc_config(self,enb=0,lc=0):
        return self.stats_data['eNB_config'][enb]['LC']['lcUeConfig'][lc]      

    # mac_stats needs to be changed to ue_status
    def get_ue_status(self,enb=0,ue=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]

    def get_num_ue(self,enb=0):
        return len(self.stats_data['mac_stats'][enb]['ue_mac_stats'])

    def get_ue_mac_status(self,enb=0,ue=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']

    def get_ue_phr(self,enb=0,ue=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['phr']
    # return for all LCIDs
    def get_ue_rlc_report(self,enb=0,ue=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['rlcReport']

    def get_num_ue_lc(self,enb=0,ue=0):
        return len(self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['rlcReport'])

    def get_ue_lc_report(self,enb=0,ue=0,lc=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['rlcReport'][lc]

    def get_ue_dlcqi_report(self,enb=0,ue=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['dlCqiReport']
   
    def get_ue_dlwbcqi(self,enb=0,ue=0,cc=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['dlCqiReport']['csiReport'][cc]['p10csi']['wbCqi']

    # lcgdi 0 for SRBs, lcgid 1 for default drb
    def get_ue_bsr(self,enb=0,ue=0, lc=0):
        if lc == 0 or lc == 1:
            return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['bsr'][0]
        elif lc == 2 :
            aggregated_bsr= self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['bsr'][1]+self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['bsr'][2]+self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['bsr'][3]
            return aggregated_bsr
        else :
            return 0

    def get_ue_harq(self,enb=0,ue=0):
        return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['harq']

    # don't need UE
    def get_enb_sfn(self,enb=0,ue=0):
	if self.get_num_ue(enb) > 0 :
            return self.stats_data['mac_stats'][enb]['ue_mac_stats'][ue]['mac_stats']['dlCqiReport']['sfnSn']
   	else:
            return 0 
   
