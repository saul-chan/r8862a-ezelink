PREFIX = $(DESTDIR)/usr/local/clourneysemi/sigma_ca
sigma_source=Wi-FiTestSuite_PCEndpoint-Linux_v10.4-1

default: all

all: wfa_tf

wfa_tf:
	tar -xvf ../../../../dl/$(sigma_source).tar.gz
	patch -p0 < HE_TG.patch
	chmod +x $(sigma_source)/scripts/
	$(MAKE) -C $(sigma_source)

debian-package: version
	dpkg-buildpackage -us -uc -rfakeroot

debian-package-clean:
	dpkg-buildpackage -rfakeroot -Tclean

clean:
	rm -rf $(sigma_source)

install:
	@echo "install Sigma CA proxy"
	install -m 777 -d $(PREFIX)
	install -m 777 -d $(PREFIX)/tg
	install -m 644 -D sigma_ca.conf.template $(PREFIX)
	install -m 755 -D sigma_ca_proxy.py $(PREFIX)
	install -m 755 -D run_ca_proxy.sh $(PREFIX)
	install -m 755 -D start_tg.sh $(PREFIX)/tg/
	install -m 755 -D $(sigma_source)/ca/wfa_ca $(PREFIX)/tg/wfa_ca_tg
	install -m 755 -D $(sigma_source)/dut/wfa_dut $(PREFIX)/tg/wfa_dut_tg
	cp -r $(sigma_source)/scripts $(PREFIX)/tg/

version:
	./set_version.sh
