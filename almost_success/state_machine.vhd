
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
fetch_htrans,
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
--- connect signals to dmai
    dmai.address <= HADDR;
    dmai.size <= HSIZE;
    dmai.wdata <= HWDATA;
    dmai.write <= HWRITE;
    dmai.irq <= '0';
    dmai.busy <= '0';
    dmai.burst <= '0';

-----------------------------------------------------
cmdProc_NextState: process(curState,dmao,htrans) 
BEGIN 
-- add state transfer here
  CASE curState IS
    WHEN IDLE =>
      IF htrans = "10" THEN
        nextState <= fetch_htrans;
      ELSE
        nextState <= IDLE;
      END IF;
    
    WHEN fetch_htrans =>
      nextState <= Instr_fetch;
    
    WHEN Instr_fetch =>
      IF dmao.ready = '1' THEN
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
    WHEN fetch_htrans =>
      dmai.start <= '1'; 
    WHEN Instr_fetch =>
      HREADY <= '0';
      dmai.start <= '0';
      END CASE;

  END PROCESS;
-----------------------------------------------------  
END myArch;
