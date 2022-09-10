V="095"
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
	echo "<li><a href=\"/downloads/iso${V}/windows32/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/windows)
do
	echo "<li><a href=\"/downloads/iso${V}/windows/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/Haiku)
do
	echo "<li><a href=\"/downloads/iso${V}/Haiku/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/Darwin)
do
	echo "<li><a href=\"/downloads/iso${V}/Darwin/${FILE}\">${FILE}</a></li>"
done

