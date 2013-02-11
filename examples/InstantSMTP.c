#include "../lib/CServe.h"

int main(int argc, char** argv){
    InstantSMTP_Settings settings;
    settings.banner = "220 magnesiumbeta.com ESMTP Instant\n";
    
    InstantSMTP(&settings, 25, 20);
}