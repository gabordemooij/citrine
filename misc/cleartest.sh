if [ "$1" = "" ]; then
	echo "No Language Code specified."
	exit
fi
echo "Language code: "
echo $1
for EXP in $(ls ../tests/test*$1*.exp);
do
	echo "$EXP"
	rm $EXP
done
