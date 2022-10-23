library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.">"; -- overload the < operator for std_logic_vectors
use ieee.std_logic_unsigned."="; -- overload the = operator for std_logic_vectors
use ieee.numeric_std.all; 

ENTITY cm0_wrapper is
  port(
  clkm: in std_logic;
  rstn: in std_logic;
  ahbmi: in std_logic;
  ahbmo(0): out std_logic
);
end;


ARCHITECTURE Wrapper_Arch OF cm0_wrapper IS
  COMPONENT CORTEXMODS is
    port(
      HADDR : out std_logic_vector (31 downto 0);
      HSIZE : out std_logic_vector (2 downto 0);
      HTRANS : out std_logic_vector (1 downto 0);
      HWDATA : out std_logic_vector (31 downto 0);
      HWRITE : out std_logic;
      HRDATA : in std_logic_vector (31 downto 0);
      HREADY : in std_logic;
      HCLK : in std_logic;
      HRESETn : in std_logic;
    );
   END COMPONENT; 
   
   COMPONENT AHB_bridge is
     port(
       --- clock and reset ---
       clkm : in std_logic;
       rstn : in std_logic;
       
       --- AHB Mater records ---
       ahbmi : in ahb_mst_in_type;
       ahbmo : out ahb_mst_out_type;
       
       --- ARM Cortex-M0 AHB-Lite Signals ---
       HADDR : in std_logic_vector (31 downto 0);
       HSIZE : in std_logic_vector (2 downto 0);
       HTRANS : in std_logic_vector (1 downto 0);
       HWDATA: in std_logic_vector (31 downto 0);
       HWRITE : in std_logic;
       HRDATA : out std_logic_vector(31 downto 0);
       HREADY : out std_logic;
     );
   END COMPONENT;
  
  signal sig_HADDR: std_logic_vector (31 downto 0);
  signal sig_HSIZE: std_logic_vector (2 downto 0);
  signal sig_HTRANS : std_logic_vector (1 downto 0);
  signal sig_HWDATA: std_logic_vector (31 downto 0);
  signal sig_HWRITE: std_logic;
  signal sig_HRDATA: std_logic_vector (31 downto 0);
  signal sig_HREADY: std_logic;
  
  begin
    cortex: CORTEXMODS
    port map(
      HCLK => clkm,
      HRESETn => rstn,
      
      HADDR => sig_HADDR,
      HSIZE => sig_HSIZE,
      HTRANS => sig_HTRANS,
      HWDATA => sig_HWDATA,
      HWRITE => sig_HWRITE,
      HRDATA => sig_HRDATA;
      HREADY => sig_HREADY
      );

    
    AHB: AHB_bridge
    port map(
      clkm => clkm,
      rstn => rstn,
      
      ahbmi => ahbmi,
      ahbmo => ahbmo(0),

      HADDR => sig_HADDR,
      HSIZE => sig_HSIZE,
      HTRANS => sig_HTRANS,
      HWDATA => sig_HWDATA,
      HWRITE => sig_HWRITE,
      HRDATA => sig_HRDATA;
      HREADY => sig_HREADY      
    );
END Wrapper_Arch;

   