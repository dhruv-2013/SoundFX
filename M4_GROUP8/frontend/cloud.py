#Tried code for future prospects to send files wirelessly 

import paramiko

# Configuration for your Xilinx Kria board and cloud server
kria_hostname = '192'
kria_username = 'root'
kria_password = 'your_password'
local_file_path = 'final.wav'
remote_file_path = 'final'

# Initialize an SSH client
ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

try:
    # Connect to the Xilinx Kria board
    ssh.connect(kria_hostname, username=kria_username, password=kria_password)

    # Create an SCP client
    scp = ssh.open_scp()

    # Upload the file from the Kria board to the cloud server
    scp.put(local_file_path, remote_file_path)

    # Close the SCP client
    scp.close()

    # Close the SSH connection
    ssh.close()

    print(f"File '{local_file_path}' transferred to '{remote_file_path}' successfully.")
except Exception as e:
    print(f"An error occurred: {str(e)}")
