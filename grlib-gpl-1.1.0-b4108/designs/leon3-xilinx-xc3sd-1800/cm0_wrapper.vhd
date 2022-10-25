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

ENTITY cm0_wrapper is
  port(
  clkm: in std_logic;
  rstn: in std_logic;
  ahbmi: in ahb_mst_in_type;
  ahbmo: out ahb_mst_out_type;
  cm0_led: out std_logic
);
end;


ARCHITECTURE Wrapper_Arch OF cm0_wrapper IS
  COMPONENT CORTEXM0DS is
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
     
      HRESP: in std_logic;
      NMI:in std_logic;
      IRQ:in std_logic_vector (15 downto 0);
      RXEV:in std_logic
     
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
       HREADY : out std_logic
     );
   END COMPONENT;
 
  signal sig_HADDR: std_logic_vector (31 downto 0);
  signal sig_HSIZE: std_logic_vector (2 downto 0);
  signal sig_HTRANS : std_logic_vector (1 downto 0);
  signal sig_HWDATA: std_logic_vector (31 downto 0);
  signal sig_HWRITE: std_logic;
  signal sig_HRDATA: std_logic_vector (31 downto 0);
  signal sig_HREADY: std_logic;
 
  signal sig_IRQ: std_logic_vector (15 downto 0):=(others => '0');
 
  signal sig_LED : std_logic;

 
  begin
   
    CORTEX:CORTEXM0DS
    port map(
      HCLK => clkm,
      HRESETn => rstn,
     
      HADDR => sig_HADDR,
      HSIZE => sig_HSIZE,
      HTRANS => sig_HTRANS,
      HWDATA => sig_HWDATA,
      HWRITE => sig_HWRITE,
      HRDATA => sig_HRDATA,
      HREADY => sig_HREADY,
     
      HRESP => '0',
      NMI => '0',
      IRQ => sig_IRQ,
      RXEV => '0'
      );

   
    AHB:AHB_bridge
    port map(
      clkm => clkm,
      rstn => rstn,
     
      ahbmi => ahbmi,
      ahbmo => ahbmo,

      HADDR => sig_HADDR,
      HSIZE => sig_HSIZE,
      HTRANS => sig_HTRANS,
      HWDATA => sig_HWDATA,
      HWRITE => sig_HWRITE,
      HRDATA => sig_HRDATA,
      HREADY => sig_HREADY      
    );
   
--begin
  LED_blink : process(sig_HRDATA,clkm)
  begin
    if (falling_edge(clkm)) then
      if sig_HRDATA(31 downto 0) = "00001010000010100000101000001010" then
        sig_LED <= '0';
      else
        sig_LED <= '1';
      end if;
    end if;
  end process;
  cm0_led <= sig_LED;

END Wrapper_Arch;

----------------------------------
