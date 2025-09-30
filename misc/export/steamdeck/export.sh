flatpak-builder --force-clean build org.citrinelang.yml
flatpak-builder --repo=repo --force-clean build org.citrinelang.yml
flatpak build-bundle repo org.citrinelang.flatpak org.citrinelang.export