primary_host="kvalobs1 kvalobs2"
ipalias_choices="/usr/local/sbin/alias_kvalobs\
                 /usr/local/sbin/alias_kvalobsnew"
ipalias=""

for myalias in $ipalias_choices; do
	if [ -x "$myalias" ]; then
		ipalias=$myalias
		break
	fi
fi


log()
{
	local mylog=""
	
    if [ -z "$logfile" ]; then
		mylog="/dev/null"
    else
		mylog=$logfile
    fi
    
	echo "$@"  | tee -a $mylog
}


ipalias_status()
{
    local me=`uname -n`
    local res=0
    local mylog=""

    if [ -z "$logfile" ]; then
		mylog="/dev/null"
    else
		mylog=$logfile
    fi


    if [ -z "$ipalias" ]; then
		echo "$me, is an test machine" >> $logfile
		echo "test"
		return 0
    fi

	$ipalias status > /dev/null 2>&1 || res=$?
	
    if [ "$res" = "2" -o "$res" = "0" ]; then
		echo "This machine has the kvalobs ip alias." >> $logfile
		echo "true" 
		return 0
    elif [ "$res" = "1" ]; then
		echo "Another machine has the kvalobs ip alias." >> $logfile
		echo "false"
		return 1
    else
		echo "FAILED - kvalobs ipalias status." >> $logfile
		echo "undefined"
		return 2
    fi
}
