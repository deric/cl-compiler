#!/usr/bin/perl

use WWW::Mechanize;
use Term::ReadKey;

$urlraco = "https://examens.fib.upc.es/examens/servlet/practiques.ExamenPractica";
$dades = "entrega.info";
$arxiuentrega = "entrega.tar";
$arxiunota = "nota";

if (-e $arxiuentrega) {

	if (-e $dades) {
		open(DADES, $dades);
		$nomusuari=<DADES>;
		chop($nomusuari);
		$password=<DADES>;
		chop($password);
		$opcio=<DADES>;
		chop($opcio);
		close(DADES);

		my $mechanize = WWW::Mechanize->new(autocheck => 1);
		$mechanize->get($urlraco);

		$mechanize->submit_form( fields => { username => $nomusuari, password => $password } );
		die unless ($mechanize->success);
		
		my $page = $mechanize->content;

		if ($page =~ /Username o password incorrectes/) {
			print "Username o password incorrectos\n";
		}
		else {
			if ($page =~ /No tens un examen/) {
				print "En este momento no hay ningun examen en esta aula.\n";
			}
			else{

				$usuari = `/assig/cl1/recuperaid`;
				@usuariaux = split(/,/, $usuari);
				$usuari = $usuariaux[0];

				$comentari = "";
				if(-e $arxiunota){
					open(NOTA,"$arxiunota");
					$nota=<NOTA>;
					close(NOTA);
					$comentari = "Nota actual: $nota     Usuario: $usuari";	
				}
				else {
					$comentari= "Usuario: $usuari";
				}
	
				$mechanize->submit_form( fields => { arxiu => $arxiuentrega,
								num => $opcio, 
								comentari => $comentari  } );
				die unless ($mechanize->success);
		
				$page = $mechanize->content;
	
				if ($page =~ /lliurat correctament/) {

	
					print "El usuario $usuari ha entragado la práctica correctamente con nota $nota\n"
				}
				else {
					if ($page =~ /ha passat la data/) {
						print "La practica se ha entregado fuera de tiempo.\n"
					}
					else {
						print "Error al entregar la practica.\n"
					}
				}
			}
		}
	}
	else{
		
		print "Escriu el teu username:"; 
		$nomusuari=<STDIN>;
		chop($nomusuari);
		print "Escriu el teu password:";
		ReadMode 'noecho';
		$password=<STDIN>;
		chop($password);
		ReadMode 0;
		print "\n";
		
		my $mechanize = WWW::Mechanize->new(autocheck => 1);
		
		$mechanize->get($urlraco);

		$mechanize->submit_form( fields => { username => $nomusuari, password => $password } );
		die unless ($mechanize->success);
		
		my $page = $mechanize->content;
	
		if ($page =~ /Username o password incorrectes/) {
			print "Username o password incorrectos\n";
		}
		else { 
			if ($page =~ /No tens un examen/) {
				print "En este momento no hay ningun examen en esta aula.\n";
			}
			else{	
				@lineasopcions = split(/<SELECT name=num>/,$page);

				@lineas = split(/\n/, $lineasopcions[1]);
				$index=1;

				$seleccionada=0;
	
				print "Escoge opcion:\n";
				foreach $linea (@lineas){
					if ($linea =~ /OPTION value=/) {

						@opcio=split(/>/,$linea);
						print "$index -> $opcio[1]";
						@valor=split(/"/,$linea);
						push(@taulavalors, $valor[1]);

						if ($linea =~ /SELECTED/){
							$seleccionada = $index;
							print "    (por defecto, ENTER)\n";
						}
						else{
							print "\n";
						}

						$index++;
					}
				}
				$opcioselec=<STDIN>;
				chop($opcioselec);

				if ($opcioselec == 0){
					$opcioselec=$seleccionada;
				}

				$usuari = `/assig/cl1/recuperaid`;
				@usuariaux = split(/,/, $usuari);
				$usuari = $usuariaux[0];

				$comentari = "";
				if(-e $arxiunota){
					open(NOTA,"$arxiunota");
					$nota=<NOTA>;
					close(NOTA);
					$comentari = "Nota actual: $nota     Usuario: $usuari";	
				}
				else {
					$comentari= "Usuario: $usuari";
				}
	
				$mechanize->submit_form( fields => { arxiu => $arxiuentrega, 
							num => $taulavalors[$opcioselec-1],
							comentari => $comentari } );
				die unless ($mechanize->success);
		
				$page = $mechanize->content;
				if ($page =~ /lliurat correctament/) {
	
					open(DADES,"+>$dades");
					print DADES "$nomusuari\n$password\n$taulavalors[$opcioselec-1]\n";
					close(DADES);
	
					print "El usuario $usuari ha entragado la práctica correctamente con nota $nota\n"
				}
				else {
					if ($page =~ /ha passat la data/) {
						print "La practica se ha entregado fuera de tiempo.\n"
					}
					else {
						print "Error al entregar la practica.\n"
					}
				}
		
			}
		}
	}
}
else{
	print "Archivo $arxiuentrega no encontrado. No se podrá realizar la entrega.\n"
}