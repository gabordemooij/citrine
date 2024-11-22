
adapt files/folders to suit your needs!

AndroidStudioProjects
├── CitrineAndroid
│   ├── app
│   │   ├── build/...
│   │   ├── build.gradle.kts
│   │   ├── jni
│   │   │   ├── Android.mk (here, write contents: include $(call all-subdir-makefiles) )
│   │   │   ├── Application.mk (here, put the file from this dir)
│   │   │   ├── Citrine
│   │   │   │   ├── Android.mk (here, put the file from this dir)
│   │   │   │   ├── citrine.c (here, put the file from this dir)
│   │   │   │   ├── orig -> link to Citrine source dir
│   │   │   ├── SDL/.. (copy of SDL2/SDL - sorry...)
│   │   │   ├── SDL2/..
│   │   │   ├── SDL_image/..
│   │   │   ├── SDL_mixer/..
│   │   │   └── SDL_ttf/..
│   │   ├── libs/..
│   │   ├── obj/..
│   │   ├── proguard-rules.pro (this file can be empty)
│   │   └── src
│   │       └── main
│   │           ├── AndroidManifest.xml
│   │           ├── assets/data (put your data pack here)
│   │           ├── java/com/citrine/citrineandroid/MainActivity.Java
│   │           └── res/... (your icons etc..)
│   ├── build.gradle
│   ├── gradle/...
│   │   └── wrapper
│   │       ├── gradle-wrapper.jar
│   │       └── gradle-wrapper.properties
│   ├── gradle.properties
│   ├── gradlew
│   ├── gradlew.bat
│   ├── local.properties
│   ├── settings.gradle
