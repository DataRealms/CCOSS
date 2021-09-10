#! /bin/bash

link_base_files() {
    if [[ ! -d $base_data_path ]]; then
        exit 1
    fi

    ln -s "${base_data_path}"/*.rte "${base_data_path}/Credits.txt" $tmp_dir
}

link_user_files() {
    local user_files=("LogConsole.txt" "LogLoading.txt" "LogLoadingWarning.txt" "AbortScreen.bmp" "AbortScreen.png" "Settings.ini")
    local user_directories=("Metagames.rte" "Scenes.rte" "_ScreenShots")

    if ! [[ -d "${user_data}" ]]; then
        mkdir -p "${user_data}"
    fi

    for file in $user_files; do
        ln -s "${user_data}/$file" $tmp_dir
    done

    if ! [[ -d "${user_data}/Metagames.rte" ]]; then
        mkdir -p "${user_data}/Metagames.rte"
        echo -e "DataModule\n\tModuleName = Metagame Saves" > "${user_data}/Metagames.rte/Index.ini"
    fi

    if ! [[ -d "${user_data}/Scenes.rte" ]]; then
        mkdir -p "${user_data}/Scenes.rte"
        echo -e "DataModule\n\tModuleName = Saves" > "${user_data}/Scenes.rte/Index.ini"
    fi

    for directory in ${user_directories[@]}; do
        if ! [[ -d "${user_data}/${directory}" ]]; then
            mkdir -p "${user_data}/${directory}"
        fi
        ln -sf "${user_data}/${directory}" "${tmp_dir}/"
    done

    for file in ${user_files[@]}; do
        ln -sf "${user_data}/${file}" "${tmp_dir}/"
    done
}
tmp_dir=$(mktemp -d "/tmp/CCCP.XXXXXXXXX")
base_data_path=@BASEDATAPATH@
user_local_home=${XDG_DATA_HOME:-~/".local/share"}
user_data="${user_local_home}/Cortex Command"

link_base_files
link_user_files

cd "${tmp_dir}"

export CCCP_SETTINGSPATH="Settings.ini"

@EXENAME@ $@

exit_code=$?

rm -r "${tmp_dir}"

exit $exit_code
