# Copyright (c) 2011-2016
#     Gabriel Hjort Blindell <ghb@kth.se>
#     George Ungureanu <ugeorge@kth.se>
#     Rodolfo Jordao <jordao@kth.se>
# All rights reserved.
#
# Redistribution and use in src and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of src code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE
# COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

unexport

SEP                     = "=============================="
export PREBUILDMSG      = "$(SEP)\n Building dependencies...\n"
export POSTBUILDMSG     = " Done.\n$(SEP)\n\n"
export ITEMBUILDMSG     = " * %\n"
export PRELINKMSG       = "$(SEP)\n Linking...\n"
export POSTLINKMSG      = $(POSTBUILDMSG)
export ITEMLINKMSG      = " * %\n"
export PREDOCSBUILDMSG  = "$(SEP)\n Building API docs...\n"
export POSTDOCSBUILDMSG = " Done.\n$(SEP)\n\n"
export PARTDOCSBUILDMSG = "%...\n"

export ROOTPATH   = ${CURDIR}/


export TARGET     = bin
export TARGETPATH = $(CURDIR)/$(TARGET)
export DOMAKE     = $(MAKE) --no-print-directory

build: bin/adse

bin/adse: dependencies
	@$(MAKE) -C ./src

dependencies: gecode boost libxml2

libxml2:
	@printf "$(SEP)\n Libxml (2)\n$(SEP)\n"
	@printf "Downloading from git and compiling right version...\n"
	git clone https://github.com/GNOME/libxml2
	cd libxml2 && git checkout f8a8c1f && sh autogen.sh --prefix=`pwd`/build
	@mkdir -p libxml2/build
	$(MAKE) -C ./libxml2
	@printf "Done.\n$(SEP)\n"

boost:
	@printf "$(SEP)\n Boost\n$(SEP)\n"
	@printf "Downloading from git and compiling right version...\n"
	git clone --recursive https://github.com/boostorg/boost.git
	@mkdir -p boost/build
	cd boost && ./bootstrap.sh --prefix=`pwd`/build && ./b2 --prefix=`pwd`/build --with-graph install
	@printf "Done.\n$(SEP)\n"

gecode:
	@printf "$(SEP)\n Gecode\n$(SEP)\n"
	@printf "Downloading from git and compiling right version...\n"
	git clone https://github.com/Gecode/gecode
	cd gecode && git checkout 1e8c55c && ./configure && $(DOMAKE) && cd ..
	@printf "Done.\n$(SEP)\n"

distclean:
	@rm -rf gecode
	@$(DOMAKE) -C ./src distclean

docs:
	@$(DOMAKE) -C ./src docs

help:
	@printf "Usage:"
	@printf
	@printf "make:       same as 'make build'"
	@printf "make build: builds the entire adse"
	@printf "make docs:  generates the Doxygen API"

$(TARGET):
	@mkdir -p $(TARGET)

clean: preclean doclean
	@printf $(POSTBUILDMSG)

preclean:
	@printf $(subst Building % module,Cleaning modules,$(PREBUILDMSG))

doclean:
	@rm -rf $(TARGET)

.PHONY: clean preclean doclean all $(TARGET) docs

