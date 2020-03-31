OS="linux"
V="0.9.2"
for FILE in $(ls downloads/linux)
do
	echo "<li><a href=\"/downloads/iso/linux/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/windows)
do
	echo "<li><a href=\"/downloads/iso/windows/${FILE}\">${FILE}</a></li>"
done