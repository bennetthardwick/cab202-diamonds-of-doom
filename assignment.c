#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <curses.h>


// Configuration
#define DELAY (16) /* Millisecond delay between game updates */

#define SHIP_WIDTH	(7)
#define SHIP_HEIGHT (3)

#define HELP_WIDTH (40)
#define HELP_HEIGHT (14)

#define DIAMOND_HEIGHT (5)
#define DIAMOND_WIDTH (5)

#define SPLINTER_FIRST_SIZE (3)
#define SPLINTER_FINAL_SIZE (1)

#define MESSAGE_WIDTH (40)
#define MESSAGE_HEIGHT (14)

#define BULLET_ARRAY_LENGTH (100)
#define BULLET_SPEED (0.3)

#define DIAMOND_ARRAY_LENGTH (10)
#define DIAMOND_SPEED (0.1)

#define SPLINTER_FIRST_ARRAY_LENGTH (20)
#define SPLINTER_FINAL_ARRAY_LENGTH (40)

#define MAX_LIVES (3)

#define GAMEOVER_WIDTH (74)
#define GAMEOVER_HEIGHT (10)

void shoot_projectile(int ke);
void spawn_bullet(double x, double y);
void spawn_diamond(double x, double y);
void draw_all_sprites();
void update_all_sprites();
void create_random_diamonds();
bool has_collided_sprite(sprite_id spr_dia, sprite_id spr);
bool has_collided_coords(sprite_id spr_dia, int x, int y);
void spawn_splinter_first(double x, double y, double dx, double dy, int angle);
void spawn_splinter_final(double x, double y, double dx, double dy, int angle);
void check_collisions();
void diamond_movement(sprite_id sprite, int width, int height);
void destroy_all_arrays();
bool sprites_exist();


// Game state.
bool game_over = false; /* Set this to true when game is over */
bool update_screen = true; /* Set to false to prevent screen update. */
bool game_over_dialog = false;

bool help_dialog = true;

char * bullet_image = 
/**/	"0";

char * ship_image =
/**/	"   .   "
/**/	"  /-\\  "
/**/	"|=\\_/=|";

char * diamond_image =
/**/	"  +  "
/**/	" *** "
/**/	"+***+"
/**/	" *** "
/**/	"  +  ";

char * diamond_3x3_image =
/**/	" + "
/**/	"+*+"
/**/	" + ";

char * diamond_1x1_image =
/**/	"+";

char * welcome_image =
/**/	"+--------------------------------------+"                                      
/**/	"|         CAB202 Assignment 1          |"                                      
/**/	"|        The Diamonds of Doom          |"                                      
/**/	"|          Bennett  Hardwick           |"                                      
/**/	"|              n9803572                |"                                      
/**/	"+--------------------------------------+"                                      
/**/	"|              Controls                |"                                      
/**/	"|      q         : quit                |"                                      
/**/	"|      h         : help                |"                                      
/**/	"| a & d / Arrows : move left/right     |"                                      
/**/	"| Space, z, x, c : shoot bullet        |"                                      
/**/	"+--------------------------------------+"                                      
/**/	"|       Press a key to play...         |"                                      
/**/	"+--------------------------------------+";

char * game_over_message =
/**/ " _______  _______  _______  _______    _______           _______  _______ "
/**/ "(  ____ \\(  ___  )(       )(  ____ \\  (  ___  )|\\     /|(  ____ \\(  ____ )"
/**/ "| (    \\/| (   ) || () () || (    \\/  | (   ) || )   ( || (    \\/| (    )|"
/**/ "| |      | (___) || || || || (__      | |   | || |   | || (__    | (____)|"
/**/ "| | ____ |  ___  || |(_)| ||  __)     | |   | |( (   ) )|  __)   |     __)"
/**/ "| | \\_  )| (   ) || |   | || (        | |   | | \\ \\_/ / | (      | (\\ (   "
/**/ "| (___) || )   ( || )   ( || (____/\\  | (___) |  \\   /  | (____/\\| ) \\ \\__"
/**/ "(_______)|/     \\||/     \\|(_______/  (_______)   \\_/   (_______/|/   \\__/"
/**/ "                                                                          "
/**/ "                       Press y to restart or n to quit                    ";

sprite_id ship;
sprite_id help;
sprite_id game_message;

sprite_id bullets[BULLET_ARRAY_LENGTH];
sprite_id diamonds[DIAMOND_ARRAY_LENGTH];
sprite_id splinter_first[SPLINTER_FIRST_ARRAY_LENGTH];
sprite_id splinter_final[SPLINTER_FINAL_ARRAY_LENGTH];

int key = -1;
int loopCount = 0;
int lives = 0;

int shoot_count;
int w, h;

double elapsed_time = 0;
double previous_time = 0;
int seconds = 0;
int minutes = 0;
int score = 0;

// Setup game.
void setup(void) {

	srand(time(NULL));

	int sw= SHIP_WIDTH, sh = SHIP_HEIGHT;
	int hw = HELP_WIDTH, hh = HELP_HEIGHT;
	int gw = GAMEOVER_WIDTH, gh = GAMEOVER_HEIGHT;
	w = screen_width(), h = screen_height();

	int x = (w - sw) / 2;
	int hx = (w - hw) / 2, hy = (h - hh) / 2;
	int gx = (w - gw) / 2, gy = (h - gh) / 2;
	ship = sprite_create(x, h - sh - 1, sw, sh, ship_image);
	help = sprite_create(hx, hy, hw, hh, welcome_image);
	game_message = sprite_create(gx, gy, gw, gh, game_over_message);

	//sprite_draw(ship);
	create_random_diamonds();
	//show_screen();
}

void draw_border(void){
	char ch = '#';
	draw_line(0, 0, 0, screen_height() - 1, ch);
	
	draw_line(0, 0, screen_width() - 1, 0, ch);
	draw_line(0, 2, screen_width() - 1, 2, ch);

	draw_line(0, screen_height() - 1, screen_width() - 1, screen_height() - 1, ch);
	draw_line(screen_width() - 1, 0, screen_width() - 1, screen_height() - 1, ch);
}

void set_globals(){
	lives = MAX_LIVES;
	previous_time = get_current_time();
	elapsed_time = 0;
	seconds = 0;
	minutes = 0;
	score = 0;
}

// Play one turn of game.
void process(void) {

	key = getch();

	if (!help_dialog && !game_over_dialog){
		int w = screen_width();
		
		draw_border();
		draw_formatted(2, 1, "Lives: %02d | Score: %03d | Timer: %02d:%02ds", lives, score, minutes, seconds);
		update_all_sprites();
		check_collisions();
		sprite_draw(ship);
		draw_all_sprites();


		elapsed_time += get_current_time() - previous_time;
		seconds = round(elapsed_time);

		if ( seconds >= 60){
			elapsed_time = 0;
			minutes++;
		}

		previous_time = get_current_time();

		int sx = round(sprite_x(ship));

		if ( key == 'h' ) help_dialog = true;
		if ( key == 'q') game_over_dialog = true;
		if ( (key == 'a' || key == KEY_LEFT) && sx > 2 ) sprite_move(ship, -1, 0);
		if ( (key == 'd' || key == KEY_RIGHT) && sx < w - sprite_width(ship) - 2 ) sprite_move(ship, +1, 0);
		if ( key == 'z' || key == 'x' || key == 'c' || key == ' ') shoot_projectile(key);
		if(lives <= 0) game_over_dialog = true;
		if(!sprites_exist()){
			destroy_all_arrays();
			setup();
		}
		
	}
	else if (game_over_dialog){
		sprite_draw(game_message);
		if(key == 'n'){
			game_over = true;
		}
		if (key == 'y'){
			destroy_all_arrays();
			set_globals();
			setup();
			game_over_dialog = false;
		}
	}
	else{	
		sprite_draw(help);
		if(key != -1){
			help_dialog = false;
			previous_time = get_current_time();
		}
	}


	//show_screen();
}

void shoot_projectile(int ke){

	switch(ke){
		case 'z':
		spawn_bullet(sprite_x(ship) + sprite_width(ship) / 2, sprite_y(ship) - 1);
		break;
		case 'x':
		spawn_bullet(sprite_x(ship), sprite_y(ship));
		spawn_bullet(sprite_x(ship) + sprite_width(ship) - 1, sprite_y(ship));
		break;
		case 'c':
		if(shoot_count > 2){
			shoot_count = 0;
		}
			switch(shoot_count){
				case 0:
					spawn_bullet(sprite_x(ship), sprite_y(ship));
					break;
				case 1:
					spawn_bullet(sprite_x(ship) + sprite_width(ship) / 2, sprite_y(ship) - 1);
					break;
				case 2:
					spawn_bullet(sprite_x(ship) + sprite_width(ship) - 1, sprite_y(ship));
					break;
			}
		shoot_count++;
		break;
		default:
		for(int i = 0; i < BULLET_ARRAY_LENGTH; i++){
			if(bullets[i]){
				break;
			}
			else if (!bullets[i] && i == BULLET_ARRAY_LENGTH - 1) spawn_bullet(sprite_x(ship) + sprite_width(ship) / 2, sprite_y(ship) - 1);
		}
	}
}

void spawn_bullet(double x, double y){
	for ( int i = 0; i < BULLET_ARRAY_LENGTH; i++){
		if (bullets[i]) {
			if ( round(sprite_x(bullets[i])) == round(x) && round(sprite_y(bullets[i])) == round(y)) break;
		}
		else if (!bullets[i]){
			bullets[i] = sprite_create(x, y, 1, 1, bullet_image);
			break;
		}
	}
}

void check_collisions(){

	for (int j = 0; j < BULLET_ARRAY_LENGTH; j++){


		for ( int i = 0; i < SPLINTER_FINAL_ARRAY_LENGTH; i++){
			if(ship && splinter_final[i] && has_collided_sprite(splinter_final[i], ship)){
				destroy_all_arrays();
				setup();
				lives--;
			}
			if(has_collided_sprite(splinter_final[i], bullets[j])){
				bullets[j] = NULL;
				splinter_final[i] = NULL;
				score++;
				j = 0;
				i = 0;
			}
		}

		for ( int i = 0; i < SPLINTER_FIRST_ARRAY_LENGTH; i++){
			if(splinter_first[i] && ship && has_collided_sprite(splinter_first[i], ship)){
				destroy_all_arrays();
				setup();
				lives--;
			}
			if(has_collided_sprite(splinter_first[i], bullets[j])){
				bullets[j] = NULL;
				spawn_splinter_final(sprite_x(splinter_first[i]), sprite_y(splinter_first[i]), sprite_dx(splinter_first[i]), sprite_dy(splinter_first[i]), 45);
				spawn_splinter_final(sprite_x(splinter_first[i]), sprite_y(splinter_first[i]), sprite_dx(splinter_first[i]), sprite_dy(splinter_first[i]), -45);
				splinter_first[i] = NULL;
				score++;
				j = 0;
				i = 0;
			}
		}

		for ( int i = 0; i < DIAMOND_ARRAY_LENGTH; i++){
			if(ship && diamonds[i] && has_collided_sprite(diamonds[i], ship)){
				destroy_all_arrays();
				setup();
				lives--;
			}
			if(has_collided_sprite(diamonds[i], bullets[j])){
				bullets[j] = NULL;	
				spawn_splinter_first(sprite_x(diamonds[i]), sprite_y(diamonds[i]), sprite_dx(diamonds[i]), sprite_dy(diamonds[i]), 45);
				spawn_splinter_first(sprite_x(diamonds[i]), sprite_y(diamonds[i]), sprite_dx(diamonds[i]), sprite_dy(diamonds[i]), -45);

				diamonds[i] = NULL;
				score++;
				j = 0;
				i = 0;
			}
		}
	}
	
}

void destroy_all_arrays(){
	for (int i = 0; i < BULLET_ARRAY_LENGTH; i++){
		if(i < DIAMOND_ARRAY_LENGTH) diamonds[i] = NULL;
		if(i < SPLINTER_FINAL_ARRAY_LENGTH) splinter_final[i] = NULL;
		if(i < SPLINTER_FIRST_ARRAY_LENGTH) splinter_first[i] = NULL;
		bullets[i] = NULL;
	}
}

bool has_collided_coords(sprite_id sprite, int x_s, int y_s){
	int x = (int) round( sprite->x );
	int y = (int) round( sprite->y );
	int offset = 0;

	for ( int row = 0; row < sprite->height; row++ ) {
		for ( int col = 0; col < sprite->width; col++ ) {
			char ch = sprite->bitmap[offset++] & 0xff;
			if ( ch != ' ' ) {
				if( x + col == x_s && y + row == y_s) return true;
			}
		}
	}
	return false;
}

bool has_collided_sprite(sprite_id sprite, sprite_id spr ){
	//assert(sprite && spr);
	if(sprite && spr){
	int x = (int) round( sprite->x );
	int y = (int) round( sprite->y );
	int offset = 0;
		for ( int row = 0; row < sprite->height; row++ ) {
			for ( int col = 0; col < sprite->width; col++ ) {
				char ch = sprite->bitmap[offset++] & 0xff;
				if ( ch != ' ' ) {
					if(has_collided_coords(spr, x + col, y + row)) return true;
				}
			}
		}
	}
	
	return false;
}

void spawn_diamond(double x, double y){
	for ( int i = 0; i < DIAMOND_ARRAY_LENGTH; i++){
		if (!diamonds[i]){
			diamonds[i] = sprite_create(x, y, DIAMOND_WIDTH, DIAMOND_HEIGHT, diamond_image);
			sprite_turn_to(diamonds[i], 0, DIAMOND_SPEED);
			sprite_turn(diamonds[i], (rand() % 90) - 45);
			break;
		}
	}
}

void spawn_splinter_first(double x, double y, double dx, double dy, int angle){
	for ( int i = 0; i < SPLINTER_FIRST_ARRAY_LENGTH; i++){
		if(!splinter_first[i]){
			splinter_first[i] = sprite_create(x, y, SPLINTER_FIRST_SIZE, SPLINTER_FIRST_SIZE, diamond_3x3_image);
			sprite_turn_to(splinter_first[i], dx, dy);
			sprite_turn(splinter_first[i], angle);
			break;
		}
	}
}

void spawn_splinter_final(double x, double y, double dx, double dy, int angle){
	for ( int i = 0; i < SPLINTER_FINAL_ARRAY_LENGTH; i++){
		if(!splinter_final[i]){
			splinter_final[i] = sprite_create(x, y, SPLINTER_FINAL_SIZE, SPLINTER_FINAL_SIZE, diamond_1x1_image);
			sprite_turn_to(splinter_final[i], dx, dy);
			sprite_turn(splinter_final[i], angle);
			break;
		}
	}
}

void update_all_sprites(){
	for ( int i = 0; i < BULLET_ARRAY_LENGTH; i++){
		if(bullets[i]){
			if(sprite_y(bullets[i]) <= 3){
				bullets[i] = NULL;
			}
			else{
				sprite_move(bullets[i], 0, -BULLET_SPEED);
			}
		}
		/*if(sprite_y(bullets[i] == 2)){
			bullets[i] = NULL;
		}*/
		if ( i < DIAMOND_ARRAY_LENGTH){
			if (diamonds[i] != NULL){
				diamond_movement(diamonds[i], DIAMOND_WIDTH, DIAMOND_HEIGHT);
			}
		}
		if ( i < SPLINTER_FIRST_ARRAY_LENGTH){
			if (splinter_first[i]){
				diamond_movement(splinter_first[i], SPLINTER_FIRST_SIZE, SPLINTER_FIRST_SIZE);
			}
		}
		if ( i < SPLINTER_FINAL_ARRAY_LENGTH){
			if (splinter_final[i]){
				diamond_movement(splinter_final[i], SPLINTER_FINAL_SIZE, SPLINTER_FINAL_SIZE);
			}
		}
	}
}

void diamond_movement(sprite_id diamond, int width, int height){
	sprite_step(diamond);

	int di_x = round(sprite_x(diamond));
	int di_y = round(sprite_y(diamond));
	double di_dx = sprite_dx(diamond);
	double di_dy = sprite_dy(diamond);

	if ( di_x <= 1) {
		di_dx = fabs(di_dx);
	}
	else if ( di_x >= w - width ) {
		di_dx = -fabs(di_dx);
	}

	if ( di_y <= 2) {
		di_dy = fabs(di_dy);
	}
	else if ( di_y >= screen_height() - height) {
		di_dy = -fabs(di_dy);
	}

	if ( di_dx != sprite_dx(diamond) || di_dy != sprite_dy(diamond) ) {
		sprite_back(diamond);
		sprite_turn_to(diamond, di_dx, di_dy);
	}
}

void create_random_diamonds(){
	for( int i = 0; i < DIAMOND_ARRAY_LENGTH; i++ ){
		spawn_diamond((rand() % (w - 2)+1), 2);
	}
}

bool sprites_exist(){
	for (int i = 0; i < BULLET_ARRAY_LENGTH; i++){
		if(i < DIAMOND_ARRAY_LENGTH) if(diamonds[i]) return true;
		if(i < SPLINTER_FINAL_ARRAY_LENGTH) if(splinter_final[i]) return true;
		if(i < SPLINTER_FIRST_ARRAY_LENGTH) if(splinter_first[i]) return true;
	}
	return false;
}

void draw_all_sprites(){

	for ( int i = 0; i < BULLET_ARRAY_LENGTH; i++){
		if(bullets[i]) sprite_draw(bullets[i]);
		if ( i < DIAMOND_ARRAY_LENGTH){
			if (diamonds[i] != NULL) sprite_draw(diamonds[i]);
		}
		if ( i < SPLINTER_FIRST_ARRAY_LENGTH){
			if (splinter_first[i]) sprite_draw(splinter_first[i]);
		}
		if ( i < SPLINTER_FINAL_ARRAY_LENGTH){
			if (splinter_final[i]) sprite_draw(splinter_final[i]);
		}
	}
}

// Clean up game
void cleanup(void) {

}

// Program entry point.
int main(void) {
	setup_screen();
	set_globals();
	setup();
	show_screen();

	while ( !game_over ) {
		clear_screen();
		process();

		if ( update_screen ) {
			show_screen();
		}

		timer_pause(DELAY);
	}

	cleanup();

	return 0;
}
