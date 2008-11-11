function destroy_old()
{
	this.destroy();
}

ScoreType = {
    parent: Clutter.Group.type,
    name: "Score",
    class_init: function(klass, prototype)
    {
		prototype.set_value = function (val)
		{
			this.value = val;
			
			var old_set = this.current_set;
			
			this.current_set = new Clutter.Group();
			this.current_set.opacity = 0;
			
			var val_s = Seed.sprintf("% 5d",val);
		
			for(var i = 0; i < val_s.length; i++)
			{
				var texture = this.num_textures[val_s[i]];
		
				if(val_s[i] == ' ')
					texture = null;
			
				var num = new Clutter.CloneTexture({parent_texture:texture});
				num.set_position(56*i-10,3);
				num.set_size(97,97);
				this.current_set.add_actor(num);
			}
			
			this.add_actor(this.current_set);

			var fadeline = new Clutter.Timeline({num_frames:30});
			var effect = Clutter.EffectTemplate._new(fadeline, Clutter.sine_inc_func);
			Clutter.effect_fade(effect, this.current_set, 255);

			if(old_set)
			{
				var deleteline = Clutter.effect_fade(effect, old_set, 0);
				Clutter.effect_scale(effect, old_set, 2, 2);
				
				deleteline.signal.completed.connect(destroy_old, old_set);
			}
			
			fadeline.start();
			
			this.bkg_top.raise_top();
		}
    },
    instance_init: function(klass)
    {
    	this.current_set = null;
    	
    	this.bkg_top = Clutter.Texture.new_from_file("./bkg_top.png");
		var bkg = Clutter.Texture.new_from_file("./bkg.png");
		var off_svg = Clutter.Texture.new_from_file("./off.svg");
		
		this.bkg_top.filter_quality = Clutter.TextureQuality.high;
		bkg.filter_quality = Clutter.TextureQuality.high;
		off_svg.filter_quality = Clutter.TextureQuality.high;
		
		this.add_actor(bkg);
		
		for(var i = 0; i < 5; i++)
		{
			var off_i = new Clutter.CloneTexture({parent_texture:off_svg});
			off_i.set_position(56*i-10,3);
			off_i.set_size(97,97);
			this.add_actor(off_i);
		}
		
		this.num_textures = new Array();
		
		for(var i = 0; i <= 9; i++)
		{
			this.num_textures[i] = Clutter.Texture.new_from_file("./"+i+".svg");
			this.num_textures[i].filter_quality = Clutter.TextureQuality.high;
		}
		
		this.bkg_top.opacity = 150;
		this.bkg_top.set_position(1,1);
		this.add_actor(this.bkg_top);
		
		this.set_value(0);
    }};

Score = new GType(ScoreType);
