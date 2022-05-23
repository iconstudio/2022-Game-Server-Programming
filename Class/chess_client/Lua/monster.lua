function plustwo(value) 
	local a
	a = 2
	return value + a
end

pos_x = 6
pos_y = plustwo(pos_x);

function test_wrapper_addnum(lhs, rhs)
	return test_addnum(lhs, rhs)
end

-- C에서는 저장된 변수의 값을 읽어와야 한다.
