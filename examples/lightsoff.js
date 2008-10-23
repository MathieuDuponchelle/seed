#!/usr/local/bin/seed

Seed.import_namespace("Gtk");

image_off = new Gtk.Image({"file": "./tim-off.svg"});
image_on = new Gtk.Image({"file": "./tim-on.svg"});

/* SxS size*/
var size = 5;

Gtk.init(null, null);
var window = new Gtk.Window({"title": "Lights Off"});
window.signal_hide.connect(Gtk.main_quit);

var wincount = 0;

var table = new Gtk.Table();
table.resize(size,size);

window.add(table);

var buttons = new Array(size);

for (i = 0; i < size; ++i)
{
	buttons[i] = new Array(size);
	for (j = 0; j < size; ++j)
	{
		buttons[i][j] = new Gtk.Button();
		buttons[i][j].lit = false;
		// everything starts out black
	
		buttons[i][j].x = i;
		buttons[i][j].y = j;

		buttons[i][j].set_image(new Gtk.Image({"pixbuf": image_off.pixbuf}));
		buttons[i][j].set_relief(Gtk.ReliefStyle.none);
		buttons[i][j].can_focus = false;
		
		buttons[i][j].signal_clicked.connect(buttoon, buttons[i][j]);
		table.attach_defaults(buttons[i][j], j, j+1, i, i+1);
	}
}
/* generate random puzzle */ 
random_clicks();

function do_click(x , y)
{
	if ( x < size - 1 )
		flip_color(x + 1, y);
	if ( x > 0 )
		flip_color(x - 1, y);
	if ( y < size - 1 )
		flip_color(x, y + 1);
	if ( y > 0 )
		flip_color(x, y - 1);

	flip_color(x,y);
}

function buttoon( button )
{
	do_click(this.x, this.y);

	if ( wincount == 0 )
	{
		Seed.print("GLORIOUS VICTORY");
		exit(0);
	}
		
}

/* simulate random clicks to generate a random but solvable puzzle */
function random_clicks()
{
	var count = Math.round(20 * Math.random());

	var sym = Math.floor(3*Math.random());

	for ( q = 0; q < count+5; ++q)
	{

	  	    
	    i = Math.round(4 * Math.random());
	    j = Math.round(4 * Math.random());
	    do_click(i, j);
	    if (sym == 0)
	    {
		    do_click(Math.abs(i-4), j);
	    }
	    else if (sym ==1)
	    {
		    do_click(Math.abs(i-4), Math.abs(j-4));
	    }
	    else
	    {
		    do_click(i,Math.abs(j-4));
	    }
	    
	}
	//do it again if you won already
	if ( wincount == 0)
		random_clicks();
}

function flip_color(i, j)
{
	if ( buttons[i][j].lit )
	{
		buttons[i][j].lit = false;
		--wincount;
		buttons[i][j].set_image(new Gtk.Image({"pixbuf": image_off.pixbuf}));
	}
	else
	{
		buttons[i][j].lit = true;
		++wincount;
		buttons[i][j].set_image(new Gtk.Image({"pixbuf": image_on.pixbuf}));
	}

}

table.show_all();
window.show_all();
Gtk.main();

