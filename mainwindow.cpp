#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_controlador(nullptr)
{
	ui->setupUi(this);
	ui->latenciaMemoriaSpinBox->setSuffix(" ciclos");
	ui->trasferenciaSpinBox->setSuffix(" ciclos");
	ui->quatumSpinBox->setSuffix(" ciclos");

	connect(ui->seleccionarArchivosButton, &QPushButton::clicked, this, &MainWindow::seleccionar_archivos);
	connect(ui->empezarSimulacionButton, &QPushButton::clicked, this, &MainWindow::empezar_simulacion);
}

MainWindow::~MainWindow() {
	delete ui;
	delete m_controlador;
}

void MainWindow::seleccionar_archivos() {
	QFileDialog dialogo(this);
	dialogo.setFileMode(QFileDialog::ExistingFiles);
	// Dialogo para escoger los archivos
	if (dialogo.exec()) m_rutas_archivos = dialogo.selectedFiles();
}

void MainWindow::empezar_simulacion() {
	Datos_usuario datos(ui->latenciaMemoriaSpinBox->value (), ui->trasferenciaSpinBox->value (), ui->quatumSpinBox->value ());
	m_controlador = new Controlador(m_rutas_archivos, datos, this);
    m_controlador->comenzar_simulacion ();
}

void MainWindow::imprimir_estado(QString estado) {
	ui->salidaSimulacion->append(estado);
}
