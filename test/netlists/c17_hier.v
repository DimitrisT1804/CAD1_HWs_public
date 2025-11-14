/////////////////////////////////////////////////////////////
// Created by: Synopsys Design Compiler(R)
// Version   : U-2022.12
// Date      : Fri Nov 14 17:52:52 2025
/////////////////////////////////////////////////////////////


module c20 ( in1, out1 );
  input in1;
  output out1;
  wire   n4;

  sg13g2_inv_1 U865 ( .A(n4), .Y(out1) );
  sg13g2_inv_1 U861 ( .A(in1), .Y(n4) );
endmodule


module c19 ( in, out );
  input in;
  output out;
  wire   n4;

  sg13g2_inv_1 U864 ( .A(in), .Y(n4) );
  c20 c20_inst ( .in1(n4), .out1(out) );
endmodule


module c18 ( in1, in2, out, out2 );
  input in1, in2;
  output out, out2;
  wire   n4;

  sg13g2_a21oi_2 U863 ( .A1(in1), .A2(in2), .B1(n4), .Y(out) );
  sg13g2_inv_1 U862 ( .A(in1), .Y(n4) );
  c19 c19_inst ( .in(n4), .out(out2) );
endmodule


module c17_hier ( N1, N2, N3, N22, N23 );
  input N1, N2, N3;
  output N22, N23;
  wire   n6, n7, n8, out1, out2, n9;

  sg13g2_nor2b_1 U6 ( .B_N(N1), .A(N2), .Y(n6) );
  sg13g2_nor2_1 U7 ( .A(n6), .B(N1), .Y(n7) );
  sg13g2_inv_1 U8 ( .A(n7), .Y(n8) );
  sg13g2_a22oi_1 U9 ( .A1(N3), .A2(n7), .B1(n6), .B2(n8), .Y(out1) );
  sg13g2_nand2_1 U10 ( .A(out1), .B(N3), .Y(out2) );
  sg13g2_nand2_1 U2 ( .A(out2), .B(n9), .Y(N23) );
  c18 c18_inst ( .in1(n6), .in2(n7), .out(n9), .out2(N22) );
endmodule

