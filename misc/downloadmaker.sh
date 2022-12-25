V="095"
for FILE in $(ls downloads/Linux)
do
	echo "<li><a href=\"/downloads/iso${V}/Linux/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/Windows32)
do
	echo "<li><a href=\"/downloads/iso${V}/Windows32/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/OpenBSD)
do
	echo "<li><a href=\"/downloads/iso${V}/OpenBSD/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/Windows64)
do
	echo "<li><a href=\"/downloads/iso${V}/Windows64/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/Darwin)
do
	echo "<li><a href=\"/downloads/iso${V}/Darwin/${FILE}\">${FILE}</a></li>"
done
for FILE in $(ls downloads/Haiku)
do
	echo "<li><a href=\"/downloads/iso${V}/Haiku/${FILE}\">${FILE}</a></li>"
done
