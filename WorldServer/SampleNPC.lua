myID = -1
myPosition = [
	x = 0,
	y = 0,
	z = 0,
]

function SetID(id:num id)
	myID = id
end

function SetPosition(pos)
	myPosition = pos
end

function Distance(pos1, pos2)
	return (pos1.x - pos2.x) ** 2 + (pos1.y - pos2.y) ** 2 + (pos1.z - pos2.z) ** 2
end


function OnPlayerMove(player_id)
	player_pos = API_get_position(player_id)

	dist = Distance(player_pos, myPosition)

	if (dist < 100) then
		API_msg_send(myID, player_id, "chat", "Hello")
	end

	return 1
end
