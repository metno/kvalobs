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


import argparse
import sys
import os.path
import time

valid_data_elements = ('seconds_since_last_data', 'last_1_minute', 'last_15_minutes', 'last_60_minutes')

def parse_args():
    parser = argparse.ArgumentParser(description="Check status of kvalobs' kafka queue")
    parser.add_argument('data', metavar='COMMAND',
                        help='Check the specified element. Valid values are ' + str(valid_data_elements))
    parser.add_argument('-c', '--critical', metavar='RANGE', action=ParseRange,
                       help='Generate critical state if number of running processes is outside this range (from:to).')
    parser.add_argument('-w', '--warning', metavar='RANGE', action=ParseRange,
                       help='Generate warning state if number of running processes is outside this range.')
    parser.add_argument('-d', '--data-file', metavar='FILE', default='/var/log/kvalobs/kafkastatus.txt',
                        help='Use an alternative status file')
    return parser.parse_args()

OK = 0
WARNING = 1
CRITICAL = 2
UNKNOWN = 3
headers = {OK: 'OK', WARNING: 'WARNING', CRITICAL: 'CRITICAL', UNKNOWN: 'UNKNOWN'}


def notify(status, msg):
    print headers[status] + ' - ' + str(msg)
    sys.exit(status)


def try_match(what, count, min, max, error_type):
    if not min <= count <= max:
        msg = '%s=%d. Expected ' % (what, count)
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


def get_value(filename, key):

    seconds_since_file_update = time.time() - os.path.getmtime(filename)
    if seconds_since_file_update > 90:
        raise Exception('Information file has not been updated')

    f = open(filename)
    for line in f:
        try:
            k, v = line.split()
            if k == key:
                if v == '-':
                    raise Exception('Data not yet present')
                return int(v)
        except ValueError: # Unable to properly parse input
            pass
    raise Exception('Unable to find data in file!')


if __name__ == '__main__':
    try:
        args = parse_args()

        if not args.data in valid_data_elements:
            notify(UNKNOWN, 'Invalid request: ' + args.data)

        value = get_value(args.data_file, args.data)

        if args.critical:
            try_match(args.data, value, args.critical[0], args.critical[1], CRITICAL)
        if args.warning:
            try_match(args.data, value, args.warning[0], args.warning[1], WARNING)
        notify(OK, '%s=%d' %(args.data, value))
    except Exception, e:
        notify(UNKNOWN, str(e))
