
myid = 99999;

function set_uid(x)
	myid = x
end

function event_player_move(player)
	player_x = API_PLAYER_GET_x(player);
	player_y = API_PLAYER_GET_y(player);
	my_x = API_get_x(myid);
	my_y = API_get_y(myid);
if(player_x == my_x) then
	if(player_y == my_y) then
		API_SendMessage(myid, player, "HELLO");
		API_Collision(myid);
		end
	end
end

function event_enemy_move(player)
API_SendMessage(myid, player,"BYE!!");
end

function event_enemy_regen(player)
API_SendMesaage(myid,player,"RESPAWN!!");
end
	
