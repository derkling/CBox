#!/bin/sh

if [ -f Makefile ]; then
	echo "Cleaning-up compilation files..."
	make distclean
fi

echo "Cleaning-up Autotools files.."
./cleanup.sh

echo "Setting-up cross-toolchain..."
. oe_init

echo "Running autogen.sh..."
./autogen.sh

echo "Configuring with --host=arm-angstrom-linux-gnueabi"
./configure --host=arm-angstrom-linux-gnueabi

echo "Configuretion done! Before compilation source the env with:"
echo ". oe_init"

