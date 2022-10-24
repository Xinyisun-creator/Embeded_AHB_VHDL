library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.">"; -- overload the < operator for std_logic_vectors
use ieee.std_logic_unsigned."="; -- overload the = operator for std_logic_vectors
use ieee.numeric_std.all; 
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;
library gaisler;
use gaisler.misc.all;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;


ENTITY State_Machine is
  Port(
      clkm : in std_logic;
      rstn : in std_logic;      
    
      HADDR : in std_logic_vector (31 downto 0);
      HSIZE : in std_logic_vector (2 downto 0);
      HTRANS : in std_logic_vector (1 downto 0);
      HWDATA : in std_logic_vector (31 downto 0);
      HWRITE : in std_logic;
      HREADY : out std_logic;
      
      dmai : out ahb_dma_in_type;
      dmao : in ahb_dma_out_type
    );
  end;
  
  
---->>>>>>>>>>>>>>>>>>>>> ARCHITECTURE <<<<<<<<<<<<<<<<<<<<<<<<----
ARCHITECTURE myArch OF State_Machine IS

TYPE state_type IS
(
IDLE,
Instr_fetch
);

TYPE logical_type is (TRUE, FALSE);
  
----------------------------------------------------- 
-----------------------------------------------------   
---add signal here
SIGNAL curState, nextState: state_type; --- states.
----------------------------------------------------- 
----------------------------------------------------- 
BEGIN
STATES_timing_process: PROCESS(clkm, rstn)
  BEGIN
    IF clkm'event and clkm='1' THEN
      curState <= nextState; 
    ELSIF rstn = '1' THEN
      curState <= IDLE;    
    END IF;
  END PROCESS; 
-----------------------------------------------------
cmdProc_NextState: process(curState,dmao,htrans) 
BEGIN 
-- add state transfer here
  CASE curState IS
    WHEN IDLE =>
<<<<<<< HEAD
      IF HTRANS = '10' THEN
=======
      IF htrans = "10" THEN
>>>>>>> 2304a5b5a7611f271e28f165e3c41829b607890a
        dmai.start <= '1';
        nextState <= Instr_fetch;
      ELSE
        nextState <= IDLE;
      END IF;
    
    WHEN Instr_fetch =>
      IF dmao.ready = '1' THEN
        HREADY <= '1';
        nextState <= IDLE;
      ELSE
        nextState <= Instr_fetch;
      END IF;
      
  END CASE;  
END PROCESS;
----------------------------------------------------- 
  STATE_output:PROCESS(curState) 
  -- add initial signal here
  BEGIN
    CASE curState IS
    WHEN IDLE =>
      HREADY <= '1';
      dmai.start <= '0';
    WHEN Instr_fetch =>
      HREADY <= '0';
      dmai.start <= '0';
      END CASE;

  END PROCESS;
-----------------------------------------------------  
END myArch;


