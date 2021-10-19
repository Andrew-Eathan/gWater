-- this is a test script don't expect it to be readable or nice
-- by AndrewEathan

require("gWaterFinal")
local running = true
gwater.Initialise(11135)
gwater.SetRunningState(true)
gwater.AddSphereCollider(Vector(0, 0, 0), 400)
gwater.AddSphereCollider(Vector(0, 0, 0), 400)
gwater.AddSphereCollider(Vector(0, 0, 0), 400)
gwater.AddSphereCollider(Vector(0, 0, -500), 400)
gwater.AddSphereCollider(Vector(0, 0, -500), 400)
gwater.AddSphereCollider(Vector(0, 0, -500), 400)
gwater.AddSphereCollider(Vector(0, 0, -500), 400)


local radius = 10

local Frame = vgui.Create( "DFrame" )
Frame:SetPos( 200, 200 ) 
Frame:SetSize( 300, 60 ) 
Frame:SetTitle( "gwater params" ) 
Frame:SetVisible( true ) 
Frame:SetDraggable( true ) 
Frame:ShowCloseButton( true ) 

local DermaNumSlider = vgui.Create( "DNumSlider", Frame )
DermaNumSlider:SetPos( 10, 30 )				-- Set the position
DermaNumSlider:SetSize( 300, 20 )			-- Set the size
DermaNumSlider:SetText( "Radius" )	-- Set the text above the slider
DermaNumSlider:SetMin( 10 )				 	-- Set the minimum number you can slide to
DermaNumSlider:SetMax( 30 )				-- Set the maximum number you can slide to
DermaNumSlider:SetDecimals( 0 )				-- Decimal places - zero for whole number
DermaNumSlider:SetValue(10)

DermaNumSlider.OnValueChanged = function( self, value )
	radius = value
	gwater.ParticleRadius(value)
end


timer.Create("spawncube", 1, 15, function()
	gwater.CreateCube(Vector(0, 0, 600), Vector(4, 4, 4), Vector())
end)

timer.Simple(20, function()
	timer.Remove("spawncube")
	gwater.Destroy()
	running = false
end)

-- this may or may have been written by mee and later mutilated by me :trollhd:
-- this doesn't work anyway, it's just a checkerboard
-- i'll fix this later
local m = CreateMaterial( "shitter01", "UnlitGeneric", {
  ["$basetexture"] = "sprites/strider_blackball", --
  ["$translucent"] = 1,
  ["$vertexalpha"] = 1
})

hook.Add("PlayerButtonDown", "bruh1", function(ply, key)
	if key == MOUSE_RIGHT then
		gwater.CleanParticles()
	end
	if key == KEY_PAD_1 then
		radius = radius - 1
		gwater.ParticleRadius(radius)
	end
	if key == KEY_PAD_2 then
		radius = radius + 1
		gwater.ParticleRadius(radius)
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
		render.DrawSprite(data[i], radius, radius, Color( 255, 255, 255 ) )
	end
end)