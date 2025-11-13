// Small circuit with nested modules to test recursive traversal

module c17_hier ( N1, N2, N3, N6, N7, N22, N23 );
  input N1, N2, N3, N6, N7;
  output N22, N23;
  wire   n6, n7, n8, out1, out_new, out2, auto;

  sg13g2_nor2b_1 U6 ( .B_N(n6), .A(n7), .Y(N23) );
  sg13g2_nor2_1 U7 ( .A(N2), .B(N7), .Y(n7) );
  sg13g2_inv_1 U8 ( .A(n8), .Y(N22) );
  sg13g2_a22oi_1 U9 ( .A1(N2), .A2(n6), .B1(N1), .B2(N3), .Y(n8) );
  sg13g2_nand2_1 U10 ( .A(N6), .B(N3), .Y(n6) );
  sg13g2_nand2_1 U2 ( .A(out1), .B(out2), .Y(out_new) );

  c18 c18_inst ( .in1(n6), .in2(n7), .in3(N7), .out(out1), .out2(out2) );
endmodule

module c18 ( in1, in2, in3, out, out2 , auto);
  input in1, in2, in3;
  output out, out2, auto;
  wire  n4;

  sg13g2_dfrbp_1 key_reg_reg_127_ ( .D(in1), .CLK(in2), .RESET_B(in3), .Q(n4) );
  sg13g2_a21oi_2 U863 ( .A1(n4), .A2(in2), .B1(in3), .Y(out) );
  sg13g2_inv_1 U862 ( .A(out2), .Y(n4) );

  c19 c19_inst ( .in1(out), .in2(in1), .in3(n4), .out(out2));
endmodule

module c19 ( in1, in2, in3, out);
  input in1, in2, in3;
  output out;
  wire  n4;

  sg13g2_inv_1 U864 ( .A(out), .Y(n4) );
  c20 c20_inst ( .in15(in1), .out15(out) );
endmodule

module c20 ( in15, out15 );
  input in15;
  output out15;
  wire  n4;

  sg13g2_inv_1 U865 ( .A(out15), .Y(n4) );
  sg13g2_inv_1 U861 ( .A(in15), .Y(out15) );
endmodule
