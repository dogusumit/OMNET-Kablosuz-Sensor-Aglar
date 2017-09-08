
package dogus;


simple baz
{
    parameters:
        int adet;
        int boyut;
    gates:
        input giris[adet];
        output cikis[adet];
}

simple sensor
{
    parameters:
        int adet;
        int boyut;
    gates:
        input giris[adet];
        output cikis[adet];

}


network ag
{
    parameters:
        int adet;
        int boyut;
        @display("bgb=500,500;bgi=background/terrain,t");
    types:
    submodules:

        sensor[adet]: sensor {
            @display("i=old/x_blank,,100");
        }

        baz: baz {
            @display("p=250,250;i=device/antennatower;is=l");
        }
    connections:
        for i=0..(adet-1), for j=0..(adet-1), if i!=j {
            sensor[i].cikis[j] --> { @display("ls=,0"); } --> sensor[j].giris[i];
        }
        for i=0..(adet-1) {
            baz.cikis[i] --> { @display("ls=,0"); } --> sensor[i].giris[i];
            baz.giris[i] <-- { @display("ls=,0"); } <-- sensor[i].cikis[i];
        }
}

