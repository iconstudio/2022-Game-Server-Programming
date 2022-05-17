function test_fun(value) 
	local a
	a = 2
	return value + a
end

pos_x = 6
pos_y = test_fun(pos_x);

-- C에서는 저장된 변수의 값을 읽어와야 한다.
