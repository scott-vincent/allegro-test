# ALLEGRO TEST

My Microsoft Flight Simulator FS2020 Instrument Panel uses the Allegro graphics library as it works on both Windows and Raspberry Pi. This is a small test program which you can build to confirm Allegro is working correctly.

# Git

On windows, download and install Git from here:

https://git-scm.com/

On Raspberry Pi, run the following command:
```
sudo apt install git
```

# Fetch Source

Clone the source code from my repository by running the following command:
```
git clone https://github.com/scott-vincent/allegro-test
```

# Build

On windows, install Microsoft Visual Studio 2022 Community Edition, open the allegro_test.sln file and build the project.

On Raspberry Pi, use the make.sh script:
```
cd allegro-test
chmod +x make.sh
./make.sh
```

# Run

On windows: 

Copy the allegro DLLs into the same folder as allegro-test.exe then double-click allegro-test.exe

On Raspberry Pi:
```
cd allegro_test
chmod +x allegro_test
./allegro_test
```
