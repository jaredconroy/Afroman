#include "Level.h"
#include "globals.h"

void Level::loadMap(const char* filename){

	map = TMXTiledMap::create(filename);
	map->retain();

}

TMXTiledMap * Level::getMap(){
	return map;
}

Point Level::tileCoordinateToPosition(Point point){

	//the tilemap coordinate system (grid) is different from Cocos' system, so we need a conversion method

	float x = floor(point.x * map->getTileSize().width * SCALE_FACTOR);
	float y = floor(point.y * map->getTileSize().height * SCALE_FACTOR);

	return Point(x, y);
}

Point Level::positionToTileCoordinate(Point point){

	//the tilemap coordinate system (grid) is different from Cocos' system, so we need a conversion method

	float x = floor(point.x / (map->getTileSize().width * SCALE_FACTOR));
	float y = floor(point.y / (map->getTileSize().height * SCALE_FACTOR));

	return Point((int)(x), (int)(y));
}

vector<Rect> Level::getTiles(Point point, int fromX, int fromY, string filename){

	//returns a list of all of the tiles in the specified layer of the map

	vector<Rect> list;

	TMXLayer *walls = map->getLayer(filename);

	int mapheight = (int)map->getMapSize().height - 1;

	for (int a = fromX; a < 2; a++) { //from "fromX" to 1 where point (the position of the player) is 0

		for (int b = fromY; b < 2; b++) {

#if defined(__EMSCRIPTEN__)
            // The concert permits reaching the map edge; neighboring probes
            // beyond the TMX bounds are empty, not valid getTileAt coordinates.
            const int tileX = (int)point.x+a, tileY = mapheight-((int)point.y+b);
            if (tileX < 0 || tileY < 0 || tileX >= map->getMapSize().width ||
                tileY >= map->getMapSize().height) continue;
#endif
			Sprite *tile = walls->getTileAt(Point((int)point.x + a, mapheight - ((int)point.y + b)));

			if (tile != NULL) {

				DrawNode *rectWithBorder = DrawNode::create();

				Vec2 vertices[] =
				{
					Vec2(0, map->getTileSize().height * SCALE_FACTOR),
					Vec2(map->getTileSize().width * SCALE_FACTOR, map->getTileSize().height * SCALE_FACTOR),
					Vec2(map->getTileSize().width * SCALE_FACTOR, 0),
					Vec2(0, 0)
				};

				Point tmp = walls->positionAt(Point((int)point.x + a, mapheight - ((int)point.y + b)));
				rectWithBorder->setPosition(tmp.x * SCALE_FACTOR, tmp.y * SCALE_FACTOR);

				Rect tileRect;

				tileRect.setRect(rectWithBorder->getBoundingBox().getMinX(), rectWithBorder->getBoundingBox().getMinY(),
								map->getTileSize().width * SCALE_FACTOR, map->getTileSize().height * SCALE_FACTOR);

				list.push_back(tileRect);
			}
		}
	}

	return list;

}

vector<int> Level::getNotes(string filename, int yValue){
	
	//returns a list of the positions of each note in the specified layer
	//the y value depends on the layer because the notes from different drums come across the screen at different y values

	vector<int> foo;

	auto layer = map->getLayer(filename);

	Size s = layer->getLayerSize();
	for (int x = 0; x < s.width; ++x)
	{
		if (layer->tileAt(Point(x, yValue)) != NULL){
			foo.push_back(x);
		}
	}

	return foo;
}

vector<Rect> Level::getWalls(string filename){

	//returns a list of the tiles in the specified layer

	vector<Rect> list;
	
	for (int i = 0; i < map->getMapSize().width; i++){
		for (int j = 0; j < map->getMapSize().height; j++){
			if (map->getLayer(filename)->getTileAt(Point(i, j)) != NULL){
				
				Rect tileRect = map->getLayer(filename)->getTileAt(Point(i, j))->getBoundingBox();

				list.push_back(tileRect);
			}
		}
	}

	return list;
}

Level::Level(void)
{
}

Level::~Level(void)
{
	map->release();
}