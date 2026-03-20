String binar(uint8_t data) {
    //String thisString = String(data, BIN);
    String thisString = "00000000" + String(data, BIN);
    return "0b" + thisString.substring(thisString.length()-8,thisString.length());
}