echo Building allegro_test
cd allegro_test
g++ allegro_test.cpp -lallegro -lallegro_image -lallegro_font -o allegro_test || exit
echo Done
