import sys
import tarfile
import os
import platform
import logging
import urllib
import time
import argparse
import shlex
import re

import subprocess
from threading import Thread

# Future proofing for python 3.4+ - imp is being deprecated, but importlib
# does not have all required functions in 2.7
import importlib
if not hasattr(importlib, 'find_loader'):
    import imp

version = (0,1)
version_str = '.'.join(str(ver) for ver in version)

DESCRIPTION= '''This script configure the raspberry Pi to act as :
 - a wifi access point
 - a database receiving data from the pollution sensors 
   (connected directly or sent remotely)
 - a web server used to deliver the date from the sensors to a web application.

 This script has only been tested on Raspbian stretch 9.4
 It should be run with sudo, as it needs to use apt to check current packages, 
 and install the ones missing.
 '''

repo = 'http://github.com/CapteursPollution/Sensor_Drivers.git'

patterns = {'distribution': re.compile(r".*Distributor ID:\\t(\w+).*Release:\\t([\d.,]+)",re.S)}
PLATFORM = sys.platform
IS_LINUX = (PLATFORM.startswith('linux'))

PROCESS_POLLING_INTERVAL = 0.1

# defined below
logger = None

def _is_root():
    return os.getuid() == 0

def is_raspbian():
    cmd = ['lsb_release', '-a']
    proc_return = _run(cmd)
    logger.debug(proc_return.aggr_stdout)
    try:
        [(DISTRIBUTION, RELEASE)] = patterns['distribution'].findall(proc_return.aggr_stdout)
    except (TypeError, ValueError) as inst:
        _exit(
                message='Error while parsing lsb_release output',
                status='unsupported_platform',
                )
    logger.info("Distribution is " + DISTRIBUTION + " " + RELEASE)
    if("Raspbian" in DISTRIBUTION):
        return 1
    else:
        return 0



def _init_logger(logger_name):
    logger = logging.getLogger(logger_name)
    handler = logging.StreamHandler(sys.stdout)
    formatter = logging.Formatter(fmt='%(asctime)s [%(levelname)s] '
                                      '[%(name)s] %(message)s',
                                  datefmt='%H:%M:%S')
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    return logger

def parse_args(args=None):
    class VerifySource(argparse.Action):
        def __call__(self, parser, args, values, option_string=None):
            setattr(args, self.dest, values)

    parser = argparse.ArgumentParser(
        description=DESCRIPTION,
        formatter_class=argparse.RawTextHelpFormatter,
    )

    verbosity_group = parser.add_mutually_exclusive_group()
    verbosity_group.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Verbose level logging to shell.',
    )
    verbosity_group.add_argument(
        '-q', '--quiet',
        action='store_true',
        help='Only print errors.',
    )

    version_group = parser.add_mutually_exclusive_group()
    version_group.add_argument(
        '-s', '--source',
        type=str,
        help='Install from the provided URL or local path.',
    )

    # Non group arguments
    parser.add_argument(
        '-f', '--force',
        action='store_true',
        help='Force install any requirements (USE WITH CARE!).',
    )

    #TODO:Maybe use this to upgrade ?
    #parser.add_argument(
        #'-u', '--upgrade',
        #action='store_true',
        #help='Upgrades Cloudify if already installed.',
    #)

    parser.add_argument(
        '--get-version',
        action='version',
        version=version_str,
    )

    # OS dependent arguments
    if IS_LINUX:
        parser.add_argument(
            '--install-pythondev',
            action='store_true',
            help='Attempt to install Python Developers Package.',
        )

    # Get args as a dict
    parsed_args = vars(parser.parse_args(args))

    return parsed_args

def _exit(message, status):
    exit_codes = {
        'unsupported_platform': 200,
        'dependency_download_failure': 220,
        'dependency_extraction_failure': 221,
        'dependency_installation_failure': 222,
        'dependency_unsupported_on_distribution': 223,
        'software_already_installed': 230,
    }

    logger.error(message)
    sys.exit(exit_codes[status])


def _run(cmd, suppress_errors=False):
    """Executes a command
    """
    logger.debug('Executing: {0}...'.format(cmd))
    pipe = subprocess.PIPE
    proc = subprocess.Popen(
        cmd, stdout=pipe, stderr=pipe)

    stderr_log_level = logging.NOTSET if suppress_errors else logging.ERROR

    stdout_thread = _PipeReader(proc.stdout, proc, logger, logging.DEBUG)
    stderr_thread = _PipeReader(proc.stderr, proc, logger, stderr_log_level)

    stdout_thread.start()
    stderr_thread.start()

 #  while proc.poll() is None:
 #      time.sleep(PROCESS_POLLING_INTERVAL)

    stdout_thread.join()
    stderr_thread.join()

    proc.aggr_stdout = stdout_thread.aggr
    proc.aggr_stderr = stderr_thread.aggr

    return proc

class _PipeReader(Thread):
    def __init__(self, fd, proc, logger, log_level):
        Thread.__init__(self)
        self.fd = fd
        self.proc = proc
        self.logger = logger
        self.log_level = log_level
        self.aggr = ''

    def run(self):
        while self.proc.poll() is None:
            output = self.fd.readline()
            if len(output) > 0:
                self.aggr += str(output)
                self.logger.log(self.log_level, output)
            else:
                time.sleep(PROCESS_POLLING_INTERVAL)


class package():
    def __init__(self, name, version):
        self.name = name
        self.version = version

    # main method, checks, asks user, download and install the package
    def install():
        if(name and version)
            _apt_install()

    #TODO : Check if the package and the right version is installed
    def check_installation():
        cmd = ""
    
    # Apt interface to perform the installation
    def _apt_install():
        cmd = ["apt-get", "install", name] #TODO


class Installer():
     #TODO: add config option
    def _init_(self):
        """constructor
        """
    def execute(self):
        """Installation Logic
        """
        logger.debug('Identified Platform: {0}'.format(PLATFORM))
        logger.debug('Identified Distribution: {0}'.format(DISTRIBUTION))
        logger.debug('Identified Release: {0}'.format(RELEASE))




logger = _init_logger(__file__)

def main():
    args = parse_args()
    if args['quiet']:
        logger.setLevel(logging.ERROR)
    elif args['verbose']:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    if not (IS_LINUX):
        _exit(
            message='Platform {0} not supported.'.format(PLATFORM),
            status='unsupported_platform',
        )
    if not is_raspbian():
        logger.warning('System detected is not Raspbian, proceed with caution')



if __name__ == '__main__':
    main()





## TODO: Maybe useful functions ##
# TODO:change to any file or change the REQUIREMENT_FILE_NAMES
#def _untar_requirement_files(archive, destination):
    #    """This will extract requirement files from an archive.
    #    """
    #    with tarfile.open(name=archive) as tar:
    #        req_files = [req_file for req_file in tar.getmembers()
    #                     if os.path.basename(req_file.name)
    #                     in REQUIREMENT_FILE_NAMES]
    #        tar.extractall(path=destination, members=req_files)


#def _download_file(url, destination):
    #    logger.info('Downloading {0} to {1}'.format(url, destination))
    #    final_url = urllib.urlopen(url).geturl()
    #    if final_url != url:
    #        logger.debug('Redirected to {0}'.format(final_url))
    #    f = urllib.URLopener()
    #    f.retrieve(final_url, destination)
