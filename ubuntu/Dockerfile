From ubuntu:19.10
COPY 19-sources.list /etc/apt/sources.list
WORKDIR /flobby
ADD flobby_installer.sh /flobby/flobby_installer.sh
RUN chmod +x /flobby/flobby_installer.sh
ENTRYPOINT ["./flobby_installer.sh"]
