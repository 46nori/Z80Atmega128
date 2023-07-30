FROM debian:12-slim

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN apt-get update \
    && groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && apt-get -y install locales \
    && localedef -f UTF-8 -i ja_JP ja_JP.UTF-8

ENV LANG ja_JP.UTF-8
ENV LANGUAGE ja_JP:ja
ENV LC_ALL ja_JP.UTF-8
ENV TZ JST-9
ENV TERM xterm

WORKDIR /src/

#
#   Basic tools
#
RUN sudo apt-get install -y less tree wget git make unzip bzip2 g++ gcc bsdmainutils

#
#   CP/M tools
#
RUN sudo apt-get install -y cpmtools

#
#   ASxxxx
#
ENV ASX_PATH="/opt/asx/bin"

RUN wget https://shop-pdp.net/_ftp/asxxxx/asxs5p40.zip \
    && unzip asxs5p40.zip \
    && rm -fr asxs5p40.zip \
    && mkdir -p ${ASX_PATH}
RUN make -C ./asxv5pxx/asxmak/linux/build -f makefile asz80 \
	&& sudo cp ./asxv5pxx/asxmak/linux/exe/asz80 ${ASX_PATH}
RUN make -C ./asxv5pxx/asxmak/linux/build -f makefile aslink \
	&& sudo cp ./asxv5pxx/asxmak/linux/exe/aslink ${ASX_PATH}

ENV PATH="${ASX_PATH}:${PATH}"

#
#   sdcc
#
RUN sudo apt-get install -y sdcc

#
#   z88dk (CP/M only)
#
ENV Z88DK_PATH="/opt/z88dk"

RUN sudo apt-get install -y m4 bison flex libxml2-dev subversion libtemplate-perl cpanminus
RUN sudo apt-get install -y ragel re2c dos2unix texinfo texi2html gdb curl cpanminus ccache libboost-all-dev libmodern-perl-perl libyaml-perl liblocal-lib-perl libcapture-tiny-perl libpath-tiny-perl libtest-differences-perl libtext-table-perl libdata-hexdump-perl libregexp-common-perl libclone-perl
RUN cpanm App::Prove Capture::Tiny::Extended CPU::Z80::Assembler Data::HexDump File::Path List::Uniq Modern::Perl Object::Tiny::RW Test::Cmd Test::Cmd::Common Test::Harness Test::HexDifferences Text::Table YAML::Tiny
RUN git clone --recursive https://github.com/z88dk/z88dk.git ${Z88DK_PATH}
RUN cpanm -l $HOME/perl5 --no-wget local::lib Template::Plugin::YAML 

RUN cd ${Z88DK_PATH} \
	&& eval "$(perl -I$HOME/perl5/lib/perl5 -Mlocal::lib)" \
    && chmod 777 build.sh \
    && BUILD_SDCC=1 BUILD_SDCC_HTTP=1 ./build.sh -p cpm

ENV PATH="${Z88DK_PATH}/bin:${PATH}" \
    ZCCCFG="${Z88DK_PATH}/lib/config/"
