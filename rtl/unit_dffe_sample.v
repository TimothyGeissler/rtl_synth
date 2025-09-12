// Virtual primitive wrapper that the converter maps directly to 74HC74

module UNIT_DFFE (
    input D,
    input CLK,
    output Q
);
    // No body; treated specially by the converter
endmodule

module unit_dffe_top (
    input clk,
    input d,
    output q
);
    UNIT_DFFE u0 (
        .D(d),
        .CLK(clk),
        .Q(q)
    );
endmodule


