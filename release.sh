#!/usr/bin/env bash

BADGETEAM_OTA_DIR="../badgeteam-ota"

if [ $# -eq 0 ]; then
  echo "Usage: ./release.sh <badge name>. For example: ./release.sh pixel."
fi

badge_name=$1
echo "Preparing new release for badge ${badge_name}"

config_file="firmware/configs/${badge_name}_defconfig"

if [ ! -f "${config_file}" ]; then
  echo "Please check the name of your badge: ${config_file} does not exist"
fi

old_version=$(grep "CONFIG_INFO_FIRMWARE_BUILD=" "${config_file}" | sed "s/=/ /" | awk '{print $2}')
old_build=$(echo "${old_version}" | cut -c 7-)

new_version="$(date +"%y%m%d")$((10#${old_build}+1))"
echo "Suggested bump from version ${old_version} > ${new_version}"

read -p "Continue and modify defconfig + run build? (Y/n)?" choice
case "$choice" in
  y|Y|"" ) ;;
  * ) exit 1;;
esac

echo "Updating defconfig"
sed -i "" "s/${old_version}/${new_version}/g" "${config_file}"

echo "Copying defconfig to sdkconfig"
cp "${config_file}" firmware/sdkconfig

echo "Running build"
./build.sh

read -p "Do you also want to copy the build into the badge.team ota repository (Y/n)?" choice
case "$choice" in
  y|Y|"" ) ;;
  * ) exit 1;;
esac

cp firmware/build/firmware.bin "${BADGETEAM_OTA_DIR}/${badge_name}.bin"
sed -i "" "s/${old_version}/${new_version}/g" "${BADGETEAM_OTA_DIR}/version/${badge_name}.txt"

echo "Release is ready to push!"