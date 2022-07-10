# Windows P2P Socket
This is a college project for the 2022.1 Computer Network class, where we needed to create a peer-to-peer file transfer UDP socket with reliable data transfer

We have two separate folders, one containing the client and the other containing the server

In order to compile both files using GCC, the following command lines must be used inside the cmd:

```
gcc cliente.c -o cliente -lws2_32
```
```
gcc servidor.c -o servidor -lws2_32
```

To run properly, you need to open one instance of "servidor.exe" and two of "cliente.exe", in this exact order. The two instances of the latter will have different roles, one of them being the seeder (uploader) and the other one being the leecher (downloader).

The command lines to execute both "servidor.exe" and "cliente.exe" are, respectively:
Just executing the "servidor.exe" file once or
```
servidor
```

```
cliente 127.0.0.1 <option>
```
The <option> field above needs to be replaced with the role number: 1 - Seeder; 2 - Leecher

If the server and clients are already opened, the leecher client will require you to write the file name and extension contained within the seeder file folder (the server just connects both clients, presuming the seeder client will have the leecher client required file).

After you type the file name and extension and press "Enter", the seeder client starts the file transfer, printing everytime a new package is sent. The leecher file will also print everytime a new package is received, printing its sequence number and checksum.

The client instances will stop after the file transfer is completed (last package sequence number printed by the leecher is -1) and the new file will be contained withing the leecher folder, with the new name being "Copia de <file_name>.<extension>"
