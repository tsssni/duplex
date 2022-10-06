#list processes with high cpu usage and kill the selected one

killtree()
{
	local PID=$1
	KILLSIG=$2
	for CHILD in $(ps -o pid --no-headers --ppid $PID); 
	do
		killtree $CHILD $KILLSIG
	done
	kill -$KILLSIG $PID
	echo "Process $PID is killed."
}

help(){
	echo -e "Usage: topkp.sh [-n] [num]\n"
	echo "Options:"
	echo "	-n	number of the processes you want to show, and the processes are sorted by cpu usage. "
	exit 0
}

set -- `getopt -o n: -l help -- "$@"`

NUM=0
while true
do
	case $1 in
		-n) 
			NUM=`expr ${2//\'/} + 1`
			shift 2
			;;
		--help)
			help
			shift
			;;
		*)
			break
	esac
done

echo
top -b -n1 | cat >a 
head -n 6 a

if [ $NUM -gt 1 ]
then
	tail -n +7 a | head -n $NUM
fi

rm a

echo -e "\nDo you want to kill any process? [Y/N] : \c"
read KILLOPT

until [ "$KILLOPT" == "Y" ] || [ "$KILLOPT" == "N" ]
do
	echo -e "\nPlease input Y or N : \c"
	read KILLOPT
done

case $KILLOPT in
	Y) echo -e "\nInput the pid : \c";
	   read KILLPID;

	   LC=$(ps --no-headers --ppid $KILLPID | wc -l);

	   if  [ "${KILLPID//[0-9]/}" != "" ] || [ $KILLPID -le 0 ]
	   then
	   		exit 1
	   fi

	   if [ $LC -gt 0 ]
	   then
		  echo -e "\nThe process has child processes. Do you want to continue? [Y/N] : \c"
		  read KILLCHD

		  until [ "$KILLCHD" == "Y" ] || [ "$KILLCHD" == "N" ]
		  do
			echo -e "\nPlease input Y or N : \c"
			read KILLCHD
		  done
		  
		  case $KILLCHD in
		  	Y) echo -e "\nThe process tree of $KILLPID :"
			   pstree $KILLPID

			   echo -e "\nInput the signal : \c"
			   read SIGKILL
			   echo

			   killtree $KILLPID $SIGKILL;;
			N) echo -e "\ntopkp.sh is terminated.\n"
			 
		  esac
	   else
	   	  echo -e "\nInput the signal : \c"
		  read KILLSIG
		  kill -"$KILLSIG" $KILLPID
		  if [ $? -eq 0 ]
		  then
		  	echo -e "\nProcess $KILLPID is killed.\n"
		  fi
	   fi
	   ;;
	N) echo -e "\ntopkp.sh is terminated.\n"
esac