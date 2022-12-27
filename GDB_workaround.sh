#!/bin/bash
if [[ $EUID -ne 0 ]]; then
    echo "$0 is not running as root. Try using sudo."
    exit 2
fi
echo Welcome, this script will help you integrate the VS Code with
echo GDB debugger so that you will be able to run and debugg with 
echo root privilages.
echo Do you want to continue ? '(Y = Yes / N = No)' 
read ch
if [[ "${ch^}" == "Y" ]]; then
	echo Executing the script
	echo Continuing to /usr/bin directory '(where GDB should be installed)'
	$cd /usr/bin
	echo Changing the name of orginal gdb to gdborig
	$mv gdb gdborig
	echo 'Creating the spoofing script for gdb'
	$(createFile)
	echo Created the gdb script
	echo Making the scritp executable
	$chmod 0755 gdb
	echo Editing entry to the sudoers file
	echo $USER	ALL=(root) NOPASSWD: /usr/bin/gdborig >> /etc/sudoers
	echo "You should be good to go..."
	exit
elif [[ "${ch^}" == "N" ]]; then
	echo "Exiting..."
	exit
fi
echo "No option chosen, exiting..."


function createFile()
{
	$cat > gdb <<'EOF'
	#!/bin/bash
	sudo gdborig "$@"
	EOF
}