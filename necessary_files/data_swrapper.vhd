library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.">"; -- overload the < operator for std_logic_vectors
use ieee.std_logic_unsigned."="; -- overload the = operator for std_logic_vectors
use ieee.numeric_std.all; 

library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;
library gaisler;
use gaisler.misc.all;


-- introduce the input & output signals
ENTITY Data_swrapper IS
PORT(
    dmao: in ahb_dma_out_type;
    HRDATA: out std_logic_vector (31 downto 0)
);
end;


-- just reverse the order of the incoming signal 
ARCHITECTURE convertor of Data_swrapper IS
begin
    HRDATA(7 downto 0) <= dmao.rdata(31 downto 24); --assign 4th input byte to 1st output
    HRDATA(15 downto 8) <= dmao.rdata(23 downto 16); --assign 3rd input byte to 2nd output
    HRDATA(23 downto 16) <= dmao.rdata(15 downto 8); --assign 2nd input byte to 3rd output
    HRDATA(31 downto 24) <= dmao.rdata(7 downto 0); --assign 1st input byte to 4th output

end;