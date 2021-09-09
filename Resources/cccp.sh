#! /bin/bash

link_base_files() {
	if [[ ! -d $base_data_path ]]; then
		exit 1
	fi

	ln -s "${base_data_path}"/*.rte "${base_data_path}/Credits.txt" $tmp_dir
}

link_user_files() {
	local user_files=("LogConsole.txt" "LogLoading.txt" "LogLoadingWarnings.txt" "AbortScreen.bmp" "AbortScreen.png" "Settings.ini")
	local user_directories=("Metagames.rte" "Scenes.rte" "_ScreenShots")

	if [[ ! -d "${user_data}" ]]; then
		mkdir -p "${user_data}"
	fi

	for file in $user_files; do
		ln -s "${user_data}/$file" $tmp_dir
	done

	if [[ ! -e "${user_data}/Metagames.rte" ]]; then
		mkdir -p "${user_data}/Metagames.rte"
		echo -e "DataModule\n\tModuleName = Metagame Saves" > "${user_data}/Metagames.rte"
	fi

	if [[ ! -e "${user_data}/Scenes.rte" ]]; then
		mkdir -p "${user_data/Scenes.rte}"
		echo -e "DataModule\n\tModuleName = Saves"
	fi

	for directory in $user_directories; do
		if [[ ! -e "${user_data}/${directory}" ]]; then
			mkdir "${user_data}/"
		fi
		ln -sf "${user_data}/${directory}" "${tmp_dir}/"
	done
}

tmp_dir=$(mktemp -d "/tmp/CCCP.XXXXXXXXX")
base_data_path=@BASEDATAPATH@
user_local_home=${XG_DATA_HOME:-~/".local/share"}
user_data="${user_local_home}/Cortex Commad"

link_base_files
link_user_files

cd "${tmp_dir}"

export CCCP_SETTINGSPATH="Settings.ini"

@EXENAME@ $@

exit_code=$?

rm -r "${tmp_dir}"

exit $exit_code
