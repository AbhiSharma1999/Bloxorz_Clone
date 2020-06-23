all: graphics

graphics: game.cpp glad.c
	g++ -o graphics game.cpp glad.c -lGL -lglfw -ldl -lao -lmpg123

clean:
	rm graphics

