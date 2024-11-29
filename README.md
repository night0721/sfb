# sfb
Suckless File Bin is a temporary file host, similar to [0x0.st](https://0x0.st) but is more minimal and simple. sfb is heavily inspired by [fiche](https://github.com/solusipse/fiche), it comes with a socket server(sfb) and socket client(fbc) that use a custom protocol(SFP, simple file protocol) which doesn't use HTTP to get around some VPS doesn't allow HTTP request to send files.

`nginx.conf` has been provided so you can do reverse proxy.  
`index.html` can be used for just nginx or flask server.

Notice for SFP, nginx is assumed to be used as index.txt will be created for each file sent(file path: OUTPUTDIR/ID/index.txt). And you are not required to use nginx for the HTTP server. With HTTP, no index.txt will be created, instead just a raw file as it will be handled by the flask server.

# Usage
```sh
sfb # SFP
pbc # SFP
python sfb.py # HTTP
```

# Dependencies
- For C Server:
None
- For Python HTTP server:
+ flask
+ flask-cors

# Building
You will need to run these with elevated privilages.
```
$ make
# make install
```

# Contributions
Contributions are welcomed, feel free to open a pull request.

# License
This project is licensed under the GNU Public License v3.0. See [LICENSE](https://github.com/night0721/sfb/blob/master/LICENSE) for more information.
