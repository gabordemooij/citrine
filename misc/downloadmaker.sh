OS="linux"
V="0.9.5"
for FILE in $(ls downloads/linux)
do
	echo "<li><a href=\"/downloads/iso/linux/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/windows)
do
	echo "<li><a href=\"/downloads/iso/windows/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/OpenBSD)
do
	echo "<li><a href=\"/downloads/iso/OpenBSD/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/windows32)
do
	echo "<li><a href=\"/downloads/iso/windows32/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/windows64)
do
	echo "<li><a href=\"/downloads/iso/windows64/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/haiku)
do
	echo "<li><a href=\"/downloads/iso/haiku/${FILE}\">${FILE}</a></li>"
done

