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
    File name: logger.py
    Author: navid nikaein
    Description: Warpper around the python logging library
    version: 1.0
    Date created: 7 July 2017
    Date last modified: 7 July 2017 
    Python Version: 2.7
'''

import io
import time
import logging


class logger(object):

    def __init__(self, log_level='info'):
        super(logger, self).__init__()
        self.log_level=log_level
       

    def init_logger(self):
    
        """initializing the pythong logger """
        logging.basicConfig(level=logging.DEBUG,
                            format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                            datefmt='%m-%d %H:%M',
                            filename='/tmp/jujuagent.log',
                            filemode='w')
        
        # define a Handler which writes INFO messages or higher to the sys.stderr
        self.console = logging.StreamHandler()
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        self.console.setFormatter(formatter)
        # add the handler to the root logger
        
        logging.getLogger('').addHandler(self.console)
        self.log = logging.getLogger('flexran_sdk')
        
        if self.log_level == 'debug':
            self.console.setLevel(logging.DEBUG)
            self.log.setLevel(logging.DEBUG)
        elif self.log_level == 'info':
            self.console.setLevel(logging.INFO)
            self.log.setLevel(logging.INFO)
        elif self.log_level == 'warn':
            self.console.setLevel(logging.WARNING)
            self.log.setLevel(logging.WARNING)
        elif self.log_level == 'error':
            self.console.setLevel(logging.ERROR)
            self.log.setLevel(logging.ERROR)
        elif self.log_level == 'critic':
            self.console.setLevel(logging.CRITICAL)
            self.log.setLevel(logging.CRITICAL)
        else:
            self.console.setLevel(logging.INFO)
            self.log.setLevel(logging.INFO)

        return self.log
