
function input = set_input(M,desc)

%    input = ones(M,1);

    if (desc == 0)
%        input(1:18) = [0; 0; 0; 1; 1; 0; 0; 0; 0; 0; 1; 1; 1; 0; 0; 0; 0; 0];
%        input(60:64) = zeros(5,1);
         input = (randi(2,M,1)==1); % to generate random logical stream
%         input(11) = 'x';
%         input(12) = 'x';
%         input(13) = 'x';
%         input(14) = 'y';
         input
         fprintf('\nWarning: Input is a logical stream, which means that any non-zero input (character), such as ''x'' or ''y'', will be mapped to a 1 before being passed to the mex function.\n');
         % input = (ones(M,1)==1)
    else
        if (M<30)
            fprintf('\nError: Please specify the number of input symbols 46 or higher.\n');
            return;
        end
        input = ones(M,1);
        input(1) = -1.55;
        input(2) = -1.55;
    	input(3) = -1;
        input(4) = -1;
    	input(5) = -1;
        input(6) = 1;
    	input(7) = 0.1;
        input(8) = 0;
    	input(9) = 3.1;
        input(10) = -0.1;
    	input(11) = 5.1;
        input(12) = 1;
    	input(13) = 2.5;
        input(14) = 0.5;
        input(15) = -2.1;
        
    	for i=16:(M-30)
            input(i) = 5;
        end
    	for i=(M-29):(M-20)
            input(i) = 1.7;
        end
        for i=(M-19):M
            input(i) = -2.3;
        end
    end 

end
    	


