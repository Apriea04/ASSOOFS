cmd_/home/osboxes/Desktop/ASSOOFS/Module.symvers := sed 's/\.ko$$/\.o/' /home/osboxes/Desktop/ASSOOFS/modules.order | scripts/mod/modpost -m -a  -o /home/osboxes/Desktop/ASSOOFS/Module.symvers -e -i Module.symvers   -T -