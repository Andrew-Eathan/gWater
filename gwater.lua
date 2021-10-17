-- this is a test script don't expect it to be readable or nice
-- by AndrewEathan

require("gWaterFinal")
local running = true
gwater.Initialise(11135)
gwater.SetRunningState(true)

timer.Create("spawncube", 1, 15, function()
	gwater.CreateCube(Vector(0, 0, -10000), Vector(4, 4, 4), VectorRand(-150, 150))
end)

timer.Simple(16, function()
	timer.Remove("spawncube")
	gwater.Destroy()
	running = false
end)

-- this may or may have been written by mee and later mutilated by me :trollhd:
-- this doesn't work anyway, it's just a checkerboard
-- i'll fix this later
local m = CreateMaterial( "shitter1", "Sprite", {
  ["$basetexture"] = "sprites/blueflare1_noz_gmod", --sprites/strider_blackball
  ["$translucent"] = 1,
  ["$vertexalpha"] = 1
})

hook.Add("KeyPress", "bruh1", function(ply, key)
	if key == IN_ATTACK2 then
		gwater.CleanParticles()
	end
end)

hook.Add("PostDrawOpaqueRenderables", "bruh", function()
	if not running then return end
	
	if input.IsMouseDown(MOUSE_FIRST) then 
		local dir = EyeVector() * 100
		local pos = EyePos() + dir
		for i=0, 10 do gwater.CreateParticle(pos + VectorRand(-10, 10), dir) end
	end
	
	local data = gwater.GetParticles()
	
	render.SetMaterial(m)
	for i=1, #data do
		render.DrawSprite(data[i], 20, 20, Color( 255, 255, 255 ) )
	end
end)