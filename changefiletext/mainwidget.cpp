#include "mainwidget.h"


#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QSettings>
#include <QSplitter>
#include <QFile>


CFT_MainWidget::CFT_MainWidget(QWidget *parent)
    :QWidget(parent),
	h_splitter(NULL),
	v_splitter(NULL)
{
    setupUi(this);
    setObjectName(QString("cft_widget"));
    initWidgets();

    resize(900, 800);
    move(100, 100);

connect(dirButton, SIGNAL(clicked()), this, SLOT(slotDir()));
connect(fileButton, SIGNAL(clicked()), this, SLOT(slotFile()));
connect(execButton, SIGNAL(clicked()), this, SLOT(slotExec()));
connect(clearButton, SIGNAL(clicked()), protocolTextEdit, SLOT(clear()));
connect(modifFilenameCheckBox, SIGNAL(clicked()), this, SLOT(slotClickedFile()));

	load();
	slotClickedFile();
}
void CFT_MainWidget::initWidgets()
{
	if (layout()) delete layout();
	QVBoxLayout *lay = new QVBoxLayout(0);
	setLayout(lay);

    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter = new QSplitter(Qt::Horizontal, this);
    lay->addWidget(v_splitter);

    h_splitter->addWidget(controlBox);
    h_splitter->addWidget(helpBox);
    v_splitter->addWidget(h_splitter);
    v_splitter->addWidget(protocolBox);
}
void CFT_MainWidget::load()
{
    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/geometry").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) restoreGeometry(ba);
    ba = settings.value(QString("%1/v_splitter").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) v_splitter->restoreState(ba);
    ba = settings.value(QString("%1/h_splitter").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) h_splitter->restoreState(ba);

    dirLineEdit->setText(settings.value(QString("%1/main_dir").arg(objectName()), QString()).toString());
    fileTypeComboBox->setEditText(settings.value(QString("%1/files_type").arg(objectName()), QString("ui")).toString());
    fileNameLineEdit->setText(settings.value(QString("%1/file_name").arg(objectName()), QString()).toString());
    text1LineEdit->setText(settings.value(QString("%1/text1").arg(objectName()), QString("old text")).toString());
    text2LineEdit->setText(settings.value(QString("%1/text2").arg(objectName()), QString("new text")).toString());

    modifFilenameCheckBox->setChecked(settings.value(QString("%1/modif_files").arg(objectName()), false).toBool());
    toEndCheckBox->setChecked(settings.value(QString("%1/to_end").arg(objectName()), false).toBool());
    autoClearCheckBox->setChecked(settings.value(QString("%1/clear_protocol").arg(objectName()), true).toBool());
    onlyFindCheckBox->setChecked(settings.value(QString("%1/only_find").arg(objectName()), true).toBool());
    prevComboBox->setCurrentIndex(settings.value(QString("%1/prev_text").arg(objectName()), 2).toInt());
    afterComboBox->setCurrentIndex(settings.value(QString("%1/after_text").arg(objectName()), 2).toInt());
}
void CFT_MainWidget::save()
{
    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/geometry").arg(objectName()), saveGeometry());
    settings.setValue(QString("%1/v_splitter").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/h_splitter").arg(objectName()), h_splitter->saveState());

    settings.setValue(QString("%1/main_dir").arg(objectName()), dirLineEdit->text());
    settings.setValue(QString("%1/files_type").arg(objectName()), fileTypeComboBox->currentText());
    settings.setValue(QString("%1/file_name").arg(objectName()), fileNameLineEdit->text());
    settings.setValue(QString("%1/text1").arg(objectName()), text1LineEdit->text());
    settings.setValue(QString("%1/text2").arg(objectName()), text2LineEdit->text());

    settings.setValue(QString("%1/modif_files").arg(objectName()), modifFilenameCheckBox->isChecked());
    settings.setValue(QString("%1/to_end").arg(objectName()), toEndCheckBox->isChecked());
    settings.setValue(QString("%1/clear_protocol").arg(objectName()), autoClearCheckBox->isChecked());
    settings.setValue(QString("%1/only_find").arg(objectName()), onlyFindCheckBox->isChecked());
    settings.setValue(QString("%1/prev_text").arg(objectName()), prevComboBox->currentIndex());
    settings.setValue(QString("%1/after_text").arg(objectName()), afterComboBox->currentIndex());
}
QString CFT_MainWidget::curDir() const
{
	QString cur_dir = dirLineEdit->text().trimmed();
	if (!cur_dir.isEmpty())
	{
		QDir dir(cur_dir);
		if (!dir.exists()) cur_dir.clear();
	}
	if (cur_dir.isEmpty()) cur_dir = QApplication::applicationDirPath();

	return cur_dir;
}
void CFT_MainWidget::slotDir()
{
	QString cur_dir = curDir();
    QString text = QFileDialog::getExistingDirectory(this, tr("Select folder"), cur_dir);
    if (!text.trimmed().isEmpty())
    	dirLineEdit->setText(text.trimmed()) ;
}
void CFT_MainWidget::slotFile()
{
	QString cur_dir = curDir();
	QFileInfo file(QFileDialog::getOpenFileName(this, tr("Open file"), cur_dir));
	if (file.exists())
		fileNameLineEdit->setText(file.fileName());
}
void CFT_MainWidget::slotExec()
{

	//testCheckMaskStruct();
	//return;


    addProtocol("////////////////////////////////////////////////////////");
    n_files_find = 0;
    n_files = 0;
    n_find = 0;
    n_changed = 0;
    if (autoClearCheckBox->isChecked()) protocolTextEdit->clear();

    QString s = dirLineEdit->text().trimmed();
    if (s.isEmpty()) {addProtocol("dir is empty!", true); return;}
    QDir dir(s);
    if (!dir.exists()) {addProtocol("dir not found!", true); return;}
    if (text1LineEdit->text().trimmed().isEmpty()) {addProtocol("enter old text!", true); return;}
    if (text2LineEdit->text().trimmed().isEmpty()) {addProtocol("enter new text!", true); return;}



    QString fname(fileNameLineEdit->text().trimmed());
    if (!fname.isEmpty())
    {
		fname = QString("%1/%2").arg(s).arg(fname);
		QFile f(fname);
		if (!f.exists(fname)) {addProtocol(QString("file [%1] not fount").arg(fname), true); return;}
		tryChange(f);
    }
    else
    {
		QString ftype(fileTypeComboBox->currentText().trimmed());
		QFileInfoList fil(dir.entryInfoList());
		for(int i=0; i<fil.count(); i++)
		{
			if (fil.at(i).isDir()) continue;
			fname = fil.at(i).fileName();
			if (ftype.isEmpty() && fname.indexOf(".") >= 0) continue;
			if (!ftype.isEmpty() && ftype != "*")
			if (fname.right(ftype.length()+1) != QString(".%1").arg(ftype)) continue;

			fname = QString("%1/%2").arg(s).arg(fname);
			QFile f(fname);
			if (!f.exists(fname)) {addProtocol(QString("file [%1] not fount").arg(fname), true); continue;}
			tryChange(f);
	//	    addProtocol(QString("%1  %2").arg(fil.at(i).fileName()).arg(fil.at(i).isDir()?"DIR":"FILE"));
		}
    }

    addProtocol(QString("RESULT: FILES %1/%2    FIND STRING %3   (was changed %4)").arg(n_files).arg(n_files_find).arg(n_find).arg(n_changed));

}
void CFT_MainWidget::addProtocol(QString s, bool isErr)
{
    QString str;
    if (isErr) str="Error: ";
    str += s;
    protocolTextEdit->append(str);
}
void CFT_MainWidget::tryChange(QFile &f, QString codec_name)
{
    addProtocol(QString("\n file - %1").arg(f.fileName()));
    QString s;

//////////////////// CHECK FILE and Get sourse string //////////////////////////////////////////////
    if (modifFilenameCheckBox->isChecked())
    {
    	s = f.fileName();
    	n_files++;
    }
    else
    {
        if (!f.open(QIODevice::ReadOnly)) {addProtocol(QString("file [%1] can't open for read").arg(f.fileName()), true); return;}
        QByteArray encodedString = f.readAll();
        f.close();
        n_files++;

        s = QString(encodedString);
        if (!codec_name.isEmpty())
        {
            QTextCodec *codec = QTextCodec::codecForName(codec_name.toLatin1());
            if (codec) s = codec->toUnicode(encodedString);
            else {addProtocol(QString("codec [%1] not found").arg(codec_name), true); return;}
        }
    }

//////////////////// FIND TEMPLATE STRINGS BY OLD MASK //////////////////////////////////////////////
    QMap<int, int> map(findOf(s));
    addProtocol(QString("Readed! Len string file %1,  found coincidence %2").arg(s.length()).arg(map.count()));
    n_find += map.count();
    if (map.count() > 0) n_files_find++;

//////////////////// TRY CHANGE //////////////////////////////////////////////                
    if (!onlyFindCheckBox->isChecked())
    {
    	changeString(s, map);
    	changeFile(s, f, codec_name);
    }
    else outFoundStrings(map, s);

    addProtocol("..........................................");
}
void CFT_MainWidget::changeFile(const QString &s, QFile &f, QString codec_name)
{
    if (modifFilenameCheckBox->isChecked())
    {
    	if (s.trimmed() == f.fileName().trimmed()) return;
    	if (!f.rename(s))
    		addProtocol(QString("Error rename file %1, bad name %2").arg(f.fileName()).arg(s));
    }
    else
    {
		if (!f.open(QIODevice::WriteOnly)) {addProtocol(QString("file [%1] can't open for write").arg(f.fileName()), true); return;}
			QTextStream stream(&f);
		stream.setCodec(codec_name.toLatin1());
			//stream.setCodec("UTF-8");
		//stream.setCodec("KOI8-R");
		stream<<s;
		f.close();
    }
}
void CFT_MainWidget::outFoundStrings(const QMap<int, int> &map, const QString &s)
{
	bool ok;
	uint prev = prevComboBox->currentText().toUInt(&ok); if (!ok) prev = 0;
	uint after = afterComboBox->currentText().toUInt(&ok); if (!ok) after = 0;
	for (int i=0; i<map.count(); i++)
	{
		QString smid(s.mid(map.keys().at(i), map.values().at(i)));

		int p1 = map.keys().at(i) - prev;
		int n1 = prev;
		if (p1 < 0) {p1=0; n1=map.keys().at(i);}

		int p2 = map.keys().at(i) + map.values().at(i);
		int n2 = after;
		if (s.length() < p2+n2) {n2 = s.length() - p2;}

		if (n1 > 0) smid = QString("%1%2").arg(s.mid(p1, n1)).arg(smid);
		if (n2 > 0) smid = QString("%1%2").arg(smid).arg(s.mid(p2, n2));

		addProtocol(smid);
	}
}
void CFT_MainWidget::changeString(QString &s, const QMap<int, int> &map)
{

	if (modifFilenameCheckBox->isChecked() && toEndCheckBox->isChecked())
	{
		addProtocol(QString("	Change: old[%1]  new[%1%2]").arg(s).arg(text2LineEdit->text()));
		n_changed++;
		s = QString("%1%2").arg(s).arg(text2LineEdit->text());
		return;
	}

//////////////////////////////////////////////////////////
    MaskInfo old_text(text1LineEdit->text());
    MaskInfo new_text(text2LineEdit->text());
    QMap<int, int>::const_iterator it = map.constEnd();
    while(it != map.constBegin())
    {
    	it--;
    	QString str(s.mid(it.key(), it.value()));
    	qDebug()<<QString("change string %1").arg(str);



    	if (new_text.isEmptyType()) str.clear();
    	else if (new_text.isSimple()) str = new_text.orig_mask;
    	else
    	{
    		int n;
    		int part_index = new_text.textParts.count() - 1;
    		int cur_index = str.length() - 1;
    		for (int p=new_text.format.length()-1; p>=0; p--)
    		{
    			if (new_text.format.mid(p, 1) == SIMPLE)
    			{
    				n = str.lastIndexOf(old_text.partMaskText(part_index), cur_index);
    				if (n >= 0)
    				{
    					str.replace(n, old_text.partMaskText(part_index).length(), new_text.partMaskText(part_index));
    					cur_index = n-1;
    				}
    				part_index--;
    			}
    			else if (new_text.format.mid(p, 1) == NUMERIC)
    			{
    				int num_size = old_text.numericInfo.at(1);
    				int num_value = old_text.numericInfo.at(2);
    				int num_count = old_text.numericInfo.at(3);

    				int j, j2=0;
    				int max_num_index = -1;
    				for (j=0; j<num_count; j++)
    				{
    					n = str.lastIndexOf(QString::number(num_value+j).rightJustified(num_size, '0'), cur_index);
    					if (n > max_num_index) {max_num_index = n; j2=j;}
    				}
    				n = -1;

    				if (max_num_index>=0)
    				{
    					QString new_snum(QString::number(new_text.numericInfo.at(2)+j2).rightJustified(new_text.numericInfo.at(1), '0'));
    					str.replace(max_num_index, num_size, new_snum);
    					cur_index = max_num_index-1;
    				}
    			}
    			else if (new_text.format.mid(p, 1) == SYMBOL)
    			{
    				cur_index--;
    			}
    			qDebug()<<QString("cur_index = %1").arg(cur_index);
    		}
     	}
		addProtocol(QString("	Change: old[%1]  new[%2]").arg(s.mid(it.key(), it.value())).arg(str));
		s.replace(it.key(), it.value(), str);
		n_changed++;
    }
}
QMap<int, int> CFT_MainWidget::findOf(const QString &s)
{	
    QMap<int, int> list;
    MaskInfo old_mask(text1LineEdit->text());
    MaskInfo new_mask(text2LineEdit->text());
    if (!old_mask.isValid()) {addProtocol("old mask is invalid format", true); return list;}
    
    if (!onlyFindCheckBox->isChecked())
    {
        if (!new_mask.isValid()) {addProtocol("new mask is invalid format", true); return list;}
		bool ok;
		compareMasks(old_mask, new_mask, ok);
		if (!ok) {addProtocol("masks format not compare", true); return list;}
    }

    parseStringByMask(list, old_mask, s);
    return list;
}
void CFT_MainWidget::parseStringByMask(QMap<int, int> &map, const MaskInfo &mask, const QString &s)
{
	map.clear();		//return;
	mask.out();

	int cur_symb_index = 0; //текущий индекс шаблона маски symbols
	int cur_begin_index = -1; //1-й индекс найденого фрагмента из строки s, подходящего под маску
    int s_pos=0;
    int index_of;

    while (s_pos < s.length())
    {
    	//qDebug()<<QString(" 	%1 %2  %3").arg(cur_symb_index).arg(cur_begin_index).arg(s_pos);
		switch (mask.symbols[cur_symb_index])
		{
			case MaskInfo::SimpleType:
			{
				index_of = s.indexOf(mask.partMaskTextByPos(cur_symb_index), s_pos);
				int part_len = mask.partMaskTextByPos(cur_symb_index).length();
				if (index_of < 0) return;

				if (cur_symb_index == 0)
				{
					cur_begin_index = index_of;
					s_pos = index_of + part_len;
					cur_symb_index = part_len;
				}
				else
				{
					if (index_of > s_pos &&
					    (mask.symbols[cur_symb_index-1] != MaskInfo::SetType || s.mid(cur_begin_index, index_of-cur_begin_index).indexOf("\n") > 0))
					{
						s_pos = cur_begin_index + 1;
						cur_symb_index = 0;
						break;
					}

					s_pos += part_len + (index_of - s_pos);
					cur_symb_index += part_len;
				}

				break;
			}
			case MaskInfo::OneType:
			{
				s_pos++;
				cur_symb_index++;
				break;
			}
			case MaskInfo::SetType:
			{
				cur_symb_index++;
				break;
			}
			case MaskInfo::NumericType:
			{
				int num_size = mask.numericInfo.at(1);
				int num_value = mask.numericInfo.at(2);
				int num_count = mask.numericInfo.at(3);
				int min_pos = -1;

				for(int i=0; i<num_count; i++)
				{
					QString s_num(QString::number(num_value+i).rightJustified(num_size, '0'));
					index_of = s.indexOf(s_num, s_pos);
					if (index_of >= 0)
					{
						if (min_pos < 0) min_pos = index_of;
						else if (index_of < min_pos)  min_pos = index_of;
					}
				}

				if (min_pos < 0) return;

				if (cur_symb_index == 0)
				{
					cur_begin_index = min_pos;
					s_pos = min_pos + num_size;
					cur_symb_index = num_size;
				}
				else
				{
					if (min_pos > s_pos &&
					    (mask.symbols[cur_symb_index-1] != MaskInfo::SetType || s.mid(cur_begin_index, min_pos-cur_begin_index).indexOf("\n") > 0))
					{
						s_pos = cur_begin_index + 1;
						cur_symb_index = 0;
						break;
					}

					s_pos += num_size + (min_pos - s_pos);
					cur_symb_index += num_size;
				}

				break;
			}
			default: {break;}
		}

		if (cur_symb_index == mask.symbols.count())
		{
	    	//qDebug()<<QString("find 	%1 %2").arg(cur_begin_index).arg(s_pos - cur_begin_index);
			map.insert(cur_begin_index, s_pos - cur_begin_index);
			cur_symb_index = 0;
			cur_begin_index = -1;
		}
    }

   // qDebug("exit parseStringByMask");
}
void CFT_MainWidget::compareMasks(const MaskInfo &mask1, const MaskInfo &mask2, bool &ok)
{
    if (onlyFindCheckBox->isChecked()) {ok=true; return;}
    
    ok = false;
    if (!mask1.isValid() || !mask2.isValid()) return;
    if (mask2.isSimple() || mask2.isEmptyType()) {ok=true; return;}
    if (mask1.isSimple() && !mask2.isSimple()) {ok=false; return;}
    if (mask1.format == mask2.format)
    {
    	if (mask1.isNumeric() && mask1.numericInfo.at(3) != mask2.numericInfo.at(3)) ok=false;
    	else ok= true;
    }
}
/////////////////////////////////////////

void CFT_MainWidget::testCheckMaskStruct()
{
	addProtocol("test mask");
    MaskInfo old_text(text1LineEdit->text());
    old_text.out();
}



