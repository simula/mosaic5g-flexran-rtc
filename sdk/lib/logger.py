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
