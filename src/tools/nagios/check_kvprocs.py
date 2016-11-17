#!/usr/bin/python
#  Kvalobs - Free Quality Control Software for Meteorological Observations
#
#  Copyright (C) 2016 met.no
#
#  Contact information:
#  Norwegian Meteorological Institute
#  Box 43 Blindern
#  0313 OSLO
#  NORWAY
#  email: kvalobs-dev@met.no
#
#  This file is part of KVALOBS
#
#  KVALOBS is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  KVALOBS is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with KVALOBS; if not, write to the Free Software Foundation Inc.,
#  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import os
import sys
import time
import argparse
import subprocess


def parse_args():
	parser = argparse.ArgumentParser(description='Check status of kvalobs processes')
	parser.add_argument('command', metavar='COMMAND',
						help='Check the specified command')
	parser.add_argument('-c', '--critical', metavar='RANGE', action=ParseRange,
	                   help='Generate critical state if number of running processes is outside this range (from:to).')
	parser.add_argument('-w', '--warning', metavar='RANGE', action=ParseRange,
	                   help='Generate warning state if number of running processes is outside this range.')
	parser.add_argument('--stopped-time-critical', metavar='MINUTES', type=int, default=45,
					help='Generate critical state if service has been stopped manually, but more than the given number of minutes ago. Default value is 45.')
	parser.add_argument('--stopped-time-warning', metavar='MINUTES', type=int, default=15,
					help='Generate warning state if service has been stopped manually, but more than the given number of minutes ago. Default value is 15.')
	return parser.parse_args()


OK = 0
WARNING = 1
CRITICAL = 2
UNKNOWN = 3
headers = {OK: 'OK', WARNING: 'WARNING', CRITICAL: 'CRITICAL', UNKNOWN: 'UNKNOWN'}

def notify(status, msg):
	print headers[status] + ' - ' + str(msg)
	sys.exit(status)

def try_match(count, min, max, error_type):
	if not min <= count <= max:
		msg = str(count) + ' processes running. Expected '
		if min == max:
			msg += 'exactly ' + str(min) + '.'
		else:
			msg += 'between %d and %d.' % (min, max)
		notify(error_type, msg)


class ParseRange(argparse.Action):
	'''Parse program argument for ranges (x:y)'''
	def __call__(self, parser, namespace, values, option_string=None):
		elements = values.split(':')
		if len(elements) == 1:
			parsed = int(elements[0]), int(elements[0])
		elif len(elements) == 2:
			parsed = int(elements[0]), int(elements[1])
			if parsed[0] > parsed[1]:
				raise Exception('Minimum > maximum for option ' + option_string)
		else:
			raise Exception('Invalid input for ' + option_string)
		setattr(namespace, self.dest, parsed)		

if __name__ == '__main__':
	try:
		args = parse_args()
		run_count = int(subprocess.check_output(['pgrep', '-xc', args.command]))
	except subprocess.CalledProcessError, e:
		if e.returncode == 1: # no match from pgrep
			run_count = 0
		else:
			notify(UNKNOWN, e)
	except Exception, e:
		notify(UNKNOWN, e)

	if run_count == 0:
		server = os.uname()[1]
		stopped_file = '/var/lib/kvalobs/run/%s-%s.stopped' % (args.command, server)
		if os.path.exists(stopped_file):
			time_since_change = (time.time() - os.path.getctime(stopped_file)) / 60
			if time_since_change > args.stopped_time_critical:
				notify(CRITICAL, args.command + ' was stopped manually long ago (%d minutes).' % (int(time_since_change),))
			elif time_since_change > args.stopped_time_warning:
				notify(WARNING, args.command + ' was stopped manually long ago (%d minutes).' % (int(time_since_change),))
			notify(OK, args.command + ' not running, but it was stopped manually.')
	
	if args.critical:
		try_match(run_count, args.critical[0], args.critical[1], CRITICAL)
	if args.warning:
		try_match(run_count, args.warning[0], args.warning[1], WARNING)
	notify(OK, '%d %s processes running' % (run_count, args.command))
