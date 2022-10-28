

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;
library gaisler;
use gaisler.misc.all;
library UNISIM;
use UNISIM.VComponents.all;


entity AHB_bridge is
  port(
 -- Clock and Reset -----------------
    clkm : in std_logic;
    rstn : in std_logic;
 -- AHB Master records --------------
    ahbmi : in ahb_mst_in_type;
    ahbmo : out ahb_mst_out_type;
 -- ARM Cortex-M0 AHB-Lite signals -- 
    HADDR : in std_logic_vector (31 downto 0); -- AHB transaction address
    HSIZE : in std_logic_vector (2 downto 0); -- AHB size: byte, half-word or word
    HTRANS : in std_logic_vector (1 downto 0); -- AHB transfer: non-sequential only
    HWDATA : in std_logic_vector (31 downto 0); -- AHB write-data
    HWRITE : in std_logic; -- AHB write control
    HRDATA : out std_logic_vector (31 downto 0); -- AHB read-data
    HREADY : out std_logic -- AHB stall signal
    
 );
end;


architecture structural of AHB_bridge is
--declare a component for state_machine
  component state_machine
    port(
      HADDR : in std_logic_vector (31 downto 0); 
      HSIZE : in std_logic_vector (2 downto 0);
      HTRANS : in std_logic_vector (1 downto 0);
      HWDATA : in std_logic_vector (31 downto 0); 
      HWRITE : in std_logic;
      HREADY : out std_logic;
      
      clkm : in std_logic;
      rstn : in std_logic;
      
      dmai : out ahb_dma_in_type;
      dmao : in ahb_dma_out_type
  );
  end component;
 
--declare a component for ahbmst 
  component ahbmst
    port(
      ahbi : in ahb_mst_in_type;
      ahbo : out ahb_mst_out_type;
      
      clk : in std_logic;
      rst : in std_logic;
      dmai : in ahb_dma_in_type;
      dmao : out ahb_dma_out_type
      
  );
  end component;
  
      

--declare a component for data_swapper 
  component data_swrapper
    port(
      HRDATA : out std_logic_vector (31 downto 0);
      dmao : in ahb_dma_out_type
      
  );
  end component;
  
      
 

-- define two new signal for connecting dmai and dmao signals between  state machine and ahbmst
  signal my_dmai : ahb_dma_in_type;
  signal my_dmao : ahb_dma_out_type;
  
  
-- define two signals called ahbi and ahbo connected between ahbmst and cm0_wrapper
  signal ahbi : ahb_mst_in_type;
  signal ahbo : ahb_mst_out_type;
   




  begin
--instantiate state_machine component and make the connections
    state : state_machine
      PORT MAP(
        HADDR => HADDR,
        HSIZE => HSIZE,
        HTRANS => HTRANS,
        HWDATA => HWDATA,
        HWRITE => HWRITE,
        HREADY => HREADY,
        clkm => clkm,
        rstn => rstn,
        dmai => my_dmai,
        dmao => my_dmao
        );
        
        
--instantiate the data_swapper component and make the connections  
    swrapper : data_swrapper
        PORT MAP(
          HRDATA => HRDATA,
          dmao => my_dmao
        );  
    
    
--instantiate the ahbmst component and make the connections 
    ahbmaster : ahbmst
        PORT MAP(
          clk => clkm,
          rst => rstn,
          ahbi => ahbmi,
          ahbo => ahbmo,
          dmao => my_dmao,
          dmai => my_dmai  
        );
    
          
      
end structural;