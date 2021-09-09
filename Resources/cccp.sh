#! /bin/bash

link_base_files()
{
		local base_data_path=@BASEDATAPATH@
		if [[ ! -d base_data_path ]]; then
				exit 1
		fi

		ln -s "${base_data_path}"/*.rte "${base_data_path}/Credits.txt" $tmp_dir
}

link_user_files()
{
		local user_files=("LogConsole.txt" "LogLoading.txt" "LogLoadingWarnings.txt" "AbortScreen.bmp" "AbortScreen.png" "Settings.ini")
		local user_directories=("Metagames.rte" "Scenes.rte" "_ScreenShots")

		if [[! -e "${user_data}"]]
			 mkdir -p "${user_data}"
		fi

		for file in $user_files; do
				ln -s "${user_data}/$file" $tmp_dir
		done

		for directory in $user_directories; do
				if [[! -e "${user_data}/${directory}"]]; then
						cp -r "${base_data_path}/${directory}" "${user_data}/"
				fi
				ln -sf "${user_data}/${directory}" "${tmp_dir}/"
		done
}

tmp_dir=$(mktemp -d CCCP.XXXXXXXXX)
user_local_home=${XG_DATA_HOME:-~/".local/share"}
user_data="${user_local_home}/Cortex Commad"

cd "${tmp_dir}"

export CCCP_SETTINGSPATH="Settings.ini"

@EXENAME@

rm -r "${tmp_dir}"
